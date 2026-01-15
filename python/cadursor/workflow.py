from dataclasses import dataclass

from cadursor.agents.executor import ExecutorAgent
from cadursor.agents.historian import HistorianAgent
from cadursor.agents.planner import PlannerAgent
from cadursor.agents.reflector import ReflectorAgent
from cadursor.agents.validator import ValidatorAgent
from cadursor.rules import RuleSet, load_rules
from cadursor.workflow_state import WorkflowState


@dataclass
class WorkflowRun:
    goal: str
    success: bool
    notes: list[str]


class WorkflowEngine:
    def __init__(self, rules_path: str | None = None, state_path: str | None = None) -> None:
        self.planner = PlannerAgent()
        self.executor = ExecutorAgent()
        self.validator = ValidatorAgent()
        self.reflector = ReflectorAgent()
        self.historian = HistorianAgent()
        self.rules: RuleSet | None = load_rules(rules_path) if rules_path else None
        self.state: WorkflowState | None = WorkflowState.load(state_path) if state_path else None

    def run(self, goal: str) -> WorkflowRun:
        plan = self.planner.create_plan(goal)
        self.historian.record("plan_created", {"goal": goal, "steps": plan.steps})

        execution = self.executor.execute(plan.steps)
        self.historian.record("execution_done", {"message": execution.message})

        validation = self.validator.validate(goal)
        self.historian.record("validation_done", {"issues": validation.issues})

        reflection = self.reflector.reflect(execution.message)
        self.historian.record("reflection_done", {"summary": reflection.summary})

        if self.state:
            self.state.add_completed(f"Workflow run: {goal}")

        success = execution.success and validation.success
        notes = validation.issues + reflection.improvements
        return WorkflowRun(goal=goal, success=success, notes=notes)
