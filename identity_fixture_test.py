"""Frozen original identity import/persist behavior; corrupt inputs never overwritten."""
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile
project=pathlib.Path(__file__).resolve().parent
root=project/'identity-fixtures'
if not root.is_dir():
    workspace=project
    for _ in range(3):workspace=workspace.parent
    root=workspace/'migration/fixtures/mldht-identity'
manifest=json.loads((root/'manifest.json').read_text())
work=pathlib.Path(tempfile.mkdtemp(prefix='biglybtc-mldht-identity-'))
restored=rejected=0
for case in manifest['cases']:
    data=(root/case['input']).read_bytes()
    assert hashlib.sha256(data).hexdigest()==case['input_sha256']
    path=work/case['input'];path.write_bytes(data);inode=path.stat().st_ino
    result=subprocess.run([sys.argv[1],str(path)],check=True,capture_output=True,timeout=10)
    if case['original_status']=='RESTORED':
        assert result.stdout==('RESTORED '+case['original_key']+'\n').encode(),case['case']
        encoded=subprocess.run([sys.argv[1],'--encode',str(path)],check=True,capture_output=True,timeout=10).stdout
        assert hashlib.sha256(encoded).hexdigest()==case['original_persisted_sha256'],case['case']
        restored+=1
    else:
        assert result.stdout.startswith(b'REJECT '),case['case']
        rejected+=1
    assert path.read_bytes()==data and path.stat().st_ino==inode
oversize=work/'oversize.input';oversize.write_bytes(b'#'*(1024*1024+1));inode=oversize.stat().st_ino
result=subprocess.run([sys.argv[1],str(oversize)],check=True,capture_output=True,timeout=10)
assert result.stdout==b'REJECT 0\n' and oversize.stat().st_ino==inode and oversize.stat().st_size==1024*1024+1
print('PASS original mlDHT identity:',restored,'restored/writer matches,',rejected,'non-destructive fallback rejections')
