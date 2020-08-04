from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize
# name表示扩展模块的名称

extensions = [Extension("*", ["*.pyx"])]
setup(
    ext_modules = cythonize(extensions)
)

# setup(
#         name = "fib",
#         ext_modules = cythonize("fib.pyx")
#)