import carbon
from pathlib import Path

REPO_PATH = Path(carbon.__file__).parent.parent
CFG_PATH = REPO_PATH / "config"
REC_PATH = REPO_PATH / "recordings"
