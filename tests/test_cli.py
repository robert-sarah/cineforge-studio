#!/usr/bin/env python3
"""Tests d'intégration pour le CLI CineForge Studio."""
import json
import os
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

class TestCineForgeCLI(unittest.TestCase):
    def setUp(self):
        self.work_dir = Path(tempfile.mkdtemp(prefix="cineforge_test_"))
        self.cli = Path(__file__).resolve().parent.parent / "build" / "cineforge-cli"
        if not self.cli.exists():
            self.skipTest("cineforge-cli n'est pas compilé.")
        
        # Créer de faux médias pour le test
        self.media_dir = self.work_dir / "media"
        self.media_dir.mkdir()
        (self.media_dir / "01_test.png").write_bytes(b"fake png data")
        (self.media_dir / "02_test.mp4").write_bytes(b"fake mp4 data")

    def tearDown(self):
        shutil.rmtree(self.work_dir, ignore_errors=True)

    def test_cli_help(self):
        result = subprocess.run([str(self.cli)], capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Usage:", result.stdout)

    def test_cli_basic_plan(self):
        # On passe un exécutable FFmpeg invalide pour que le rendu échoue vite
        # sans bloquer, et on vérifie que l'agent a bien interprété le plan avant.
        result = subprocess.run([
            str(self.cli),
            "--folder", str(self.media_dir),
            "--command", "crée un vlog"
        ], capture_output=True, text=True, env={"PATH": ""})
        self.assertIn("Plan local : style=vlog", result.stdout)

if __name__ == "__main__":
    unittest.main()
