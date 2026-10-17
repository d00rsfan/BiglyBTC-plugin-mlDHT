"""Replay source-extracted original policy observations, no JVM/network at runtime."""
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile
project = pathlib.Path(__file__).resolve().parent
root = project / 'fixtures'
if not root.is_dir():
    workspace = project
    for _ in range(3):
        workspace = workspace.parent
    root = workspace / 'migration/fixtures/mldht-policies'
manifest = json.loads((root/'manifest.json').read_text())
for name, digest in manifest['files'].items():
    assert hashlib.sha256((root/name).read_bytes()).hexdigest() == digest
work = pathlib.Path(tempfile.mkdtemp(prefix='biglybtc-mldht-policy-'))
source = work/'input.tsv'
shutil.copyfile(root/'input.tsv',source)
result = subprocess.run([sys.argv[1],str(source)],check=True,capture_output=True,timeout=30)
assert result.stdout == (root/'original.expected').read_bytes()
print('PASS original mlDHT bind/effective-state host policies:',manifest['cases'],'cases')
