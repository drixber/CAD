from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class WorkflowState:
    current_phase: str = ""
    completed_items: list[str] = field(default_factory=list)
    pending_items: list[str] = field(default_factory=list)
    decisions_made: list[str] = field(default_factory=list)
    estimated_remaining_time: str = ""

    @classmethod
    def load(cls, path: str | Path) -> "WorkflowState":
        state = cls()
        section = ""
        for raw in Path(path).read_text(encoding="utf-8").splitlines():
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            if line.startswith("current_phase:"):
                state.current_phase = line.split(":", 1)[1].strip()
                continue
            if line.endswith(":"):
                section = line[:-1]
                continue
            if line.startswith("- "):
                item = line[2:].strip()
                if section == "completed_items":
                    state.completed_items.append(item)
                elif section == "pending_items":
                    state.pending_items.append(item)
                elif section == "decisions_made":
                    state.decisions_made.append(item)
                continue
            if line.startswith("estimated_remaining_time:"):
                state.estimated_remaining_time = line.split(":", 1)[1].strip()
        return state

    def add_completed(self, item: str) -> None:
        if item not in self.completed_items:
            self.completed_items.append(item)

    def add_pending(self, item: str) -> None:
        if item not in self.pending_items:
            self.pending_items.append(item)

    def write(self, path: str | Path) -> None:
        lines = [
            "# Workflow State",
            "",
            f"current_phase: {self.current_phase}",
            "",
            "completed_items:",
        ]
        lines.extend(f"- {item}" for item in self.completed_items)
        lines.append("")
        lines.append("pending_items:")
        lines.extend(f"- {item}" for item in self.pending_items)
        lines.append("")
        lines.append("decisions_made:")
        lines.extend(f"- {item}" for item in self.decisions_made)
        lines.append("")
        lines.append(f"estimated_remaining_time: {self.estimated_remaining_time}")
        Path(path).write_text("\n".join(lines) + "\n", encoding="utf-8")
