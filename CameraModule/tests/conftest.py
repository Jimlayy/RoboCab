import sys
from pathlib import Path

# Gör att testerna kan skriva "import vision", oavsett var pytest startas.
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
