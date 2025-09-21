# commit_sha_to_influx.py
# Example Python script showing how to log latest Git commit hashes to InfluxDB

import subprocess
from datetime import datetime

def get_latest_commit():
    """Return the latest Git commit hash and timestamp."""
    commit_hash = subprocess.check_output(["git", "rev-parse", "HEAD"]).decode().strip()
    commit_time = datetime.utcnow().isoformat()
    return commit_hash, commit_time

if __name__ == "__main__":
    sha, ts = get_latest_commit()
    print(f"Latest commit: {sha} at {ts}")
