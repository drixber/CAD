from dataclasses import dataclass
from pathlib import Path


@dataclass
class Rule:
    rule_id: str
    description: str
    limit: int | None = None


@dataclass
class RuleSet:
    rules: list[Rule]

    def get(self, rule_id: str) -> Rule | None:
        for rule in self.rules:
            if rule.rule_id == rule_id:
                return rule
        return None


def load_rules(path: str | Path) -> RuleSet:
    rules: list[Rule] = []
    current: dict[str, str] = {}
    for raw in Path(path).read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("- id:"):
            if current:
                rules.append(
                    Rule(
                        rule_id=current.get("id", ""),
                        description=current.get("description", ""),
                        limit=int(current["limit"]) if "limit" in current else None,
                    )
                )
                current = {}
            current["id"] = line.split(":", 1)[1].strip()
        elif line.startswith("description:"):
            current["description"] = line.split(":", 1)[1].strip()
        elif line.startswith("limit:"):
            current["limit"] = line.split(":", 1)[1].strip()
    if current:
        rules.append(
            Rule(
                rule_id=current.get("id", ""),
                description=current.get("description", ""),
                limit=int(current["limit"]) if "limit" in current else None,
            )
        )
    return RuleSet(rules=rules)
