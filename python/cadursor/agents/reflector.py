from dataclasses import dataclass


@dataclass
class Reflection:
    summary: str
    improvements: list[str]


class ReflectorAgent:
    def reflect(self, result: str) -> Reflection:
        return Reflection(summary=result, improvements=[])
