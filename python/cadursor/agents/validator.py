from dataclasses import dataclass


@dataclass
class ValidationResult:
    success: bool
    issues: list[str]


class ValidatorAgent:
    def validate(self, goal: str) -> ValidationResult:
        return ValidationResult(success=True, issues=[])
