from dataclasses import dataclass


@dataclass
class HistoryEntry:
    event: str
    metadata: dict


class HistorianAgent:
    def __init__(self) -> None:
        self.entries: list[HistoryEntry] = []

    def record(self, event: str, metadata: dict | None = None) -> None:
        self.entries.append(HistoryEntry(event=event, metadata=metadata or {}))
