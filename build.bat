@echo off
setlocal

REM Cree le repertoire de build s'il n'existe pas
if not exist "build" (
    echo "Creation du repertoire de build..."
    mkdir build
)

REM Va dans le repertoire de build
cd build

REM Genere les fichiers de projet avec CMake
echo "Generation des fichiers de projet avec CMake..."
cmake ..

REM Compile le projet
echo "Compilation du projet..."
cmake --build .

echo.
echo "Build termine. L'executable se trouve dans le repertoire 'build\Debug' (ou 'build\Release')."
echo "Vous pouvez maintenant executer le programme avec la commande :"
echo ".\Debug\fsearch.exe \"votre_nom_de_fichier\""
echo.

endlocal
pause
