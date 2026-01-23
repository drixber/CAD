import argparse
from pathlib import Path

from cadursor.workflow import WorkflowEngine


def main() -> int:
    parser = argparse.ArgumentParser(description="Hydra CAD agent workflow runner")
    parser.add_argument("--goal", required=True, help="Goal to execute")
    parser.add_argument("--rules", default=".cursorcad", help="Rules file path")
    parser.add_argument("--state", default="workflow_state.md", help="Workflow state path")
    args = parser.parse_args()

    rules_path = str(Path(args.rules))
    state_path = str(Path(args.state))

    engine = WorkflowEngine(rules_path=rules_path, state_path=state_path)
    result = engine.run(args.goal)
    status = "success" if result.success else "failure"
    print(f"Workflow {status}: {result.goal}")
    if result.notes:
        print("Notes:")
        for note in result.notes:
            print(f"- {note}")
    return 0 if result.success else 1


if __name__ == "__main__":
    raise SystemExit(main())
