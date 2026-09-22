import pytest
import os

def pytest_addoption(parser):
    parser.addoption(
        "--she-path",
        action="store",
        default=None,
        help="Path to the executable"
    )

@pytest.fixture(autouse=True)
def she_path(request):
    return os.path.abspath(request.config.getoption("--she-path"))
