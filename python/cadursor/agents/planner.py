from dataclasses import dataclass


@dataclass
class Plan:
    goal: str
    steps: list[str]


class PlannerAgent:
    def create_plan(self, goal: str) -> Plan:
        return Plan(goal=goal, steps=["analyze", "design", "execute", "validate"])
