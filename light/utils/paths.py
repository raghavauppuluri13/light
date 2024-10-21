import light
from pathlib import Path

REPO_PATH = Path(light.__file__).parent.parent
PKG_PATH = Path(light.__file__).parent
CFG_PATH = REPO_PATH / "config"
REC_PATH = REPO_PATH / "recordings"
