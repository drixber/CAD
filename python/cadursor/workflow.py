from dataclasses import dataclass

from cadursor.agents.executor import ExecutorAgent
from cadursor.agents.historian import HistorianAgent
from cadursor.agents.planner import PlannerAgent
from cadursor.agents.reflector import ReflectorAgent
from cadursor.agents.validator import ValidatorAgent


@dataclass
class WorkflowRun:
    goal: str
    success: bool
    notes: list[str]


class WorkflowEngine:
    def __init__(self) -> None:
        self.planner = PlannerAgent()
        self.executor = ExecutorAgent()
        self.validator = ValidatorAgent()
        self.reflector = ReflectorAgent()
        self.historian = HistorianAgent()

    def run(self, goal: str) -> WorkflowRun:
        plan = self.planner.create_plan(goal)
        self.historian.record("plan_created", {"goal": goal, "steps": plan.steps})

        execution = self.executor.execute(plan.steps)
        self.historian.record("execution_done", {"message": execution.message})

        validation = self.validator.validate(goal)
        self.historian.record("validation_done", {"issues": validation.issues})

        reflection = self.reflector.reflect(execution.message)
        self.historian.record("reflection_done", {"summary": reflection.summary})

        success = execution.success and validation.success
        notes = validation.issues + reflection.improvements
        return WorkflowRun(goal=goal, success=success, notes=notes)
