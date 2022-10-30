import os
from typing import List


def get_names_with_extension(folder: str, ext: str) -> List[str]:
	names = [f for f in os.listdir(folder) if f.endswith(ext)]
	names.sort()
	return names


def remove_extension(name: str) -> str:
	tokens = name.rsplit('.', 1)
	return tokens[0]

