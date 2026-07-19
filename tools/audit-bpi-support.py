#!/usr/bin/env python3
"""Validate Banana Pi model, detection, pin-map and catalog consistency.

The script uses only the Python standard library so it can run in CI and on
minimal board images.  Run it from either repository, and pass the companion
repository with --peer to enforce cross-project parity.
"""

from __future__ import annotations

import argparse
import re
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Mapping, Sequence, Tuple


ALLOWED_STATUSES = {
    "implemented",
    "alias",
    "limited",
    "carrier-only",
    "base-covered",
    "blocked",
    "deferred",
    "needs-review",
    "out-of-scope",
}
EXPECTED_CATALOG_IDS = [
    *(f"L{number:02d}" for number in range(1, 83)),
    *(f"O{number:02d}" for number in range(1, 29)),
]


class AuditFailure(RuntimeError):
    pass


@dataclass(frozen=True)
class BoardRow:
    name: str
    model: str
    maps: Tuple[str, str, str]


@dataclass
class RepositoryData:
    root: Path
    flavor: str
    models: Dict[str, int]
    model_names: Dict[str, str]
    boards: Dict[str, BoardRow]
    valid_boards: Dict[str, BoardRow]
    arrays: Dict[str, Tuple[str, ...]]
    detection_literals: Tuple[str, ...]
    catalog: Dict[str, Tuple[str, str, str]]
    catalog_counts: Counter


def fail(message: str) -> None:
    raise AuditFailure(message)


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except OSError as error:
        fail(f"cannot read {path}: {error}")


def strip_c_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//[^\n]*", "", text)


def repository_layout(root: Path) -> Tuple[str, Path, Path, List[Path]]:
    wiring_source = root / "wiringPi" / "wiringPi_bpi.c"
    gpio_source = root / "source" / "c_gpio_bpi.c"
    if wiring_source.is_file():
        return (
            "BPI-WiringPi2",
            wiring_source,
            root / "wiringPi" / "wiringPi_bpi.h",
            sorted((root / "wiringPi" / "board").glob("*.h")),
        )
    if gpio_source.is_file():
        return (
            "RPi.GPIO",
            gpio_source,
            gpio_source,
            sorted((root / "source").glob("*.h")),
        )
    fail(f"{root} is not a supported Banana Pi GPIO repository")


def parse_models(definition_text: str) -> Tuple[Dict[str, int], int]:
    models = {
        f"BPI_MODEL_{name}": int(value)
        for name, value in re.findall(
            r"^\s*#define\s+BPI_MODEL_([A-Z0-9_]+)\s+(\d+)\s*$",
            definition_text,
            flags=re.MULTILINE,
        )
        if name != "MIN"
    }
    maximum_match = re.search(
        r"^\s*#define\s+BPI_MODELS_MAX\s+(\d+)\s*$",
        definition_text,
        flags=re.MULTILINE,
    )
    if not models or maximum_match is None:
        fail("model definitions or BPI_MODELS_MAX are missing")
    maximum = int(maximum_match.group(1))
    values = sorted(models.values())
    expected = list(range(min(values), maximum))
    if values != expected:
        fail(f"model IDs are not contiguous: got {values}, expected {expected}")
    if maximum != max(values) + 1:
        fail(f"BPI_MODELS_MAX={maximum}, expected {max(values) + 1}")
    return models, maximum


def parse_model_names(text: str) -> Dict[str, str]:
    return dict(
        re.findall(
            r"\[\s*(BPI_MODEL_[A-Z0-9_]+)\s*\]\s*=\s*\"([^\"]+)\"",
            text,
        )
    )


def board_table_block(text: str) -> str:
    match = re.search(
        r"struct\s+BPIBoards\s+bpiboard\s*\[\s*\]\s*=\s*\{(.*?)^\}\s*;",
        text,
        flags=re.DOTALL | re.MULTILINE,
    )
    if match is None:
        fail("cannot locate bpiboard[]")
    return match.group(1)


def parse_boards(text: str, models: Mapping[str, int]) -> Tuple[Dict[str, BoardRow], Dict[str, BoardRow]]:
    boards: Dict[str, BoardRow] = {}
    valid: Dict[str, BoardRow] = {}
    for line in board_table_block(text).splitlines():
        match = re.match(r'^\s*\{\s*"([^"]+)"\s*,(.*)$', line)
        if match is None:
            continue
        fields = [field.strip() for field in match.group(2).split(",")]
        if len(fields) < 9:
            fail(f"malformed bpiboard[] row: {line.strip()}")
        name = match.group(1)
        if name in boards:
            fail(f"duplicate detection name: {name}")
        map_names = tuple(
            match.group(0)
            for field in fields[6:9]
            if (match := re.match(r"[A-Za-z_][A-Za-z0-9_]*", field)) is not None
        )
        if len(map_names) != 3:
            fail(f"cannot parse pin-map fields in bpiboard[] row: {line.strip()}")
        row = BoardRow(name=name, model=fields[1], maps=map_names)
        boards[name] = row
        if row.model.startswith("BPI_MODEL_"):
            if row.model not in models:
                fail(f"{name} uses undefined model {row.model}")
            valid[name] = row

    if not valid:
        fail("bpiboard[] has no Banana Pi model rows")
    used_models = {row.model for row in valid.values()}
    missing_models = sorted(set(models) - used_models)
    if missing_models:
        fail(f"models without bpiboard[] rows: {', '.join(missing_models)}")

    maps_by_model: Dict[str, Tuple[str, str, str]] = {}
    for row in valid.values():
        previous = maps_by_model.setdefault(row.model, row.maps)
        if previous != row.maps:
            fail(
                f"aliases for {row.model} select different maps: "
                f"{previous} versus {row.maps} ({row.name})"
            )
    return boards, valid


def parse_arrays(header_paths: Iterable[Path]) -> Dict[str, Tuple[str, ...]]:
    arrays: Dict[str, Tuple[str, ...]] = {}
    expression = re.compile(
        r"(?:\bconst\s+)?\bint\s+([A-Za-z_][A-Za-z0-9_]*)\s*"
        r"\[\s*64\s*\]\s*=\s*\{(.*?)\}\s*;",
        flags=re.DOTALL,
    )
    for path in header_paths:
        text = strip_c_comments(read_text(path))
        for name, body in expression.findall(text):
            values = tuple(
                re.sub(r"\s+", "", item)
                for item in body.split(",")
                if item.strip()
            )
            if name in arrays and arrays[name] != values:
                fail(f"conflicting definitions for pin array {name}")
            arrays[name] = values
    return arrays


def extract_function(text: str, function_name: str) -> str:
    signature = text.find(function_name)
    if signature < 0:
        fail(f"cannot locate {function_name}()")
    opening = text.find("{", signature)
    if opening < 0:
        fail(f"cannot locate body of {function_name}()")
    depth = 0
    for position in range(opening, len(text)):
        if text[position] == "{":
            depth += 1
        elif text[position] == "}":
            depth -= 1
            if depth == 0:
                return text[opening : position + 1]
    fail(f"unterminated {function_name}()")


def parse_detection_literals(text: str) -> Tuple[str, ...]:
    body = strip_c_comments(extract_function(text, "bpi_find_board_by_model_string"))
    r4_mini = body.find('"BPI-R4 Mini"')
    r4_generic = body.find('return bpi_find_board_by_name("bpi-r4")')
    if (
        r4_mini < 0
        or r4_generic < 0
        or r4_mini > r4_generic
        or "return NULL" not in body[r4_mini:r4_generic]
    ):
        fail("R4 Mini exclusion must precede the generic R4 substring match")
    r3_mini = body.find('"BPI-R3 Mini"')
    r3_generic = body.find('return bpi_find_board_by_name("bpi-r3")')
    if (
        r3_mini < 0
        or r3_generic < 0
        or r3_mini > r3_generic
        or "return NULL" not in body[r3_mini:r3_generic]
    ):
        fail("R3 Mini exclusion must precede the generic R3 substring match")
    return tuple(re.findall(r'"([^"]+)"', body))


def parse_catalog(path: Path) -> Tuple[Dict[str, Tuple[str, str, str]], Counter]:
    text = read_text(path)
    metadata = re.search(r"<!--\s*catalog-status-counts:\s*(.*?)\s*-->", text)
    if metadata is None:
        fail(f"{path} lacks catalog-status-counts metadata")
    declared = Counter(
        {
            status: int(count)
            for status, count in re.findall(r"([a-z-]+)=(\d+)", metadata.group(1))
        }
    )
    rows: Dict[str, Tuple[str, str, str]] = {}
    for line in text.splitlines():
        match = re.match(
            r"^\|\s*([LO]\d{2})\s*\|\s*(.*?)\s*\|\s*(.*?)\s*\|\s*`([a-z-]+)`\s*\|",
            line,
        )
        if match is None:
            continue
        board_id, product, platform, status = match.groups()
        if board_id in rows:
            fail(f"duplicate catalog ID {board_id}")
        if status not in ALLOWED_STATUSES:
            fail(f"unknown catalog status {status} for {board_id}")
        rows[board_id] = (product, platform, status)

    if list(rows) != EXPECTED_CATALOG_IDS:
        missing = sorted(set(EXPECTED_CATALOG_IDS) - set(rows))
        extra = sorted(set(rows) - set(EXPECTED_CATALOG_IDS))
        fail(f"catalog IDs/order differ; missing={missing}, extra={extra}")
    actual = Counter(status for _, _, status in rows.values())
    if actual != declared:
        fail(f"catalog counts differ: declared={dict(declared)}, actual={dict(actual)}")
    return rows, actual


def validate_wiki_links(wiki_dir: Path) -> None:
    pages = {path.stem for path in wiki_dir.glob("*.md")}
    broken: List[str] = []
    for path in sorted(wiki_dir.glob("*.md")):
        for target in re.findall(r"\[[^]]+\]\(([^)]+)\)", read_text(path)):
            target = target.split("#", 1)[0]
            if not target or re.match(r"^[a-z][a-z0-9+.-]*:", target, flags=re.IGNORECASE):
                continue
            if "/" in target or target.endswith(".md"):
                if not (path.parent / target).resolve().is_file():
                    broken.append(f"{path.name}: {target}")
            elif target not in pages:
                broken.append(f"{path.name}: {target}")
    if broken:
        fail(f"broken relative Wiki links: {', '.join(broken)}")


def load_repository(root: Path) -> RepositoryData:
    root = root.resolve()
    flavor, source_path, model_path, header_paths = repository_layout(root)
    source_text = read_text(source_path)
    models, _maximum = parse_models(read_text(model_path))
    model_name_path = root / "wiringPi" / "wiringPi.c" if flavor == "BPI-WiringPi2" else source_path
    model_names = parse_model_names(read_text(model_name_path))
    if set(model_names) != set(models):
        missing = sorted(set(models) - set(model_names))
        extra = sorted(set(model_names) - set(models))
        fail(f"model-name coverage differs; missing={missing}, extra={extra}")
    boards, valid_boards = parse_boards(source_text, models)
    arrays = parse_arrays(header_paths)
    used_arrays = {array for row in valid_boards.values() for array in row.maps}
    missing_arrays = sorted(used_arrays - set(arrays))
    if missing_arrays:
        fail(f"bpiboard[] references missing arrays: {', '.join(missing_arrays)}")
    wrong_sizes = sorted((name, len(arrays[name])) for name in used_arrays if len(arrays[name]) != 64)
    if wrong_sizes:
        fail(f"pin arrays must contain 64 entries: {wrong_sizes}")
    wiki_dir = root / "docs" / "wiki"
    validate_wiki_links(wiki_dir)
    catalog, catalog_counts = parse_catalog(wiki_dir / "Complete-Board-Catalog.md")
    return RepositoryData(
        root=root,
        flavor=flavor,
        models=models,
        model_names=model_names,
        boards=boards,
        valid_boards=valid_boards,
        arrays=arrays,
        detection_literals=parse_detection_literals(source_text),
        catalog=catalog,
        catalog_counts=catalog_counts,
    )


def board_map_values(data: RepositoryData, row: BoardRow) -> Tuple[Tuple[str, ...], ...]:
    return tuple(data.arrays[name] for name in row.maps)


def compare_peer(local: RepositoryData, peer: RepositoryData) -> None:
    if local.flavor == peer.flavor:
        fail(f"--peer must name the companion repository, not another {local.flavor} tree")
    if local.models != peer.models:
        fail("model definitions differ between repositories")
    if local.model_names != peer.model_names:
        fail("model display names differ between repositories")
    if set(local.boards) != set(peer.boards):
        missing = sorted(set(local.boards) - set(peer.boards))
        extra = sorted(set(peer.boards) - set(local.boards))
        fail(f"detection-name sets differ; peer_missing={missing}, peer_extra={extra}")
    for name in local.boards:
        left = local.boards[name]
        right = peer.boards[name]
        if left.model != right.model:
            fail(f"{name} selects {left.model} locally but {right.model} in peer")
        if left.model.startswith("BPI_MODEL_") and board_map_values(local, left) != board_map_values(peer, right):
            fail(f"pin-map values differ for detection name {name}")
    if local.detection_literals != peer.detection_literals:
        fail("model/compatible detection patterns or their ordering differ between repositories")
    if local.catalog != peer.catalog:
        fail("complete board catalogs differ between repositories")


def format_counts(counts: Counter) -> str:
    return ", ".join(f"{status}={counts[status]}" for status in sorted(counts))


def main(argv: Sequence[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--peer", type=Path, help="path to the companion GPIO repository")
    arguments = parser.parse_args(argv)
    try:
        local = load_repository(arguments.root)
        print(f"PASS {local.flavor}: {local.root}")
        print(f"  model IDs: {len(local.models)} ({min(local.models.values())}-{max(local.models.values())})")
        print(
            f"  detection names: {len(local.boards)} total, "
            f"{len(local.valid_boards)} product/model aliases"
        )
        print(f"  pin arrays referenced: {len({name for row in local.valid_boards.values() for name in row.maps})}")
        print(f"  catalog: {len(local.catalog)} products ({format_counts(local.catalog_counts)})")
        if arguments.peer is not None:
            peer = load_repository(arguments.peer)
            compare_peer(local, peer)
            print(f"PASS cross-repository parity: {local.flavor} == {peer.flavor}")
        return 0
    except AuditFailure as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
