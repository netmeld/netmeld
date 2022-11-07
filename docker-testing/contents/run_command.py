from re import sub
import subprocess

def run_command(**kwargs):
    expected_return_codes = [0]
    if "return" in kwargs:
        expected_return_codes =  kwargs["return"]
        del kwargs["return"]
    print(f"Running {kwargs}, expecting one of {expected_return_codes}")
    kwargs["capture_output"] = True
    
    r = subprocess.run(**kwargs)
    output = str(r.stdout, "utf8")
    err = str(r.stderr, "utf8")
    print("======================")
    print(r.returncode)
    print("OUT: " + output)
    print("ERR: " + err)
    print("======================")
    success = r.returncode in expected_return_codes
    if not success:
        print(f"Command returned unexpected status: {r.returncode}")
    else:
        print(f"Command returned expected status: {r.returncode}")

    return success
