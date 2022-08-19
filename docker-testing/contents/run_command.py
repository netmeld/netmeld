import subprocess

def run_command(**kwargs):
    expected_return_codes = [0, 1, 80]
    if "return" in kwargs:
        expected_return_codes =  kwargs["return"]
        del kwargs["return"]
    print(f"Running {kwargs}, expecting one of {expected_return_codes}")
    r = subprocess.run(**kwargs)
    print(r.returncode)
    success = r.returncode in expected_return_codes
    if not success:
        print(f"Command returned unexpected status: {r.returncode}")
    else:
        print(f"Command returned expected status: {r.returncode}")

    return success
