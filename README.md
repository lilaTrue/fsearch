# fsearch - Recherche de Fichiers Ultra-Rapide

`fsearch` est un utilitaire en ligne de commande pour Windows, conçu pour trouver des fichiers sur l'ensemble de vos disques durs avec une vitesse maximale. Il tire parti du multi-threading et d'optimisations de bas niveau pour minimiser le temps de recherche, même sur des systèmes avec un grand nombre de fichiers.

![Exemple de sortie de fsearch](https://i.imgur.com/exemple.png)  <!-- Remplacer par une vraie capture d'écran si possible -->

## Fonctionnalités

- **Recherche Multi-threadée :** Utilise tous les cœurs de votre processeur pour paralléliser la recherche.
- **Haute Performance :** Optimisé pour réduire la contention des verrous et maximiser l'utilisation des ressources système.
- **Interface Simple :** Une seule commande, pas d'options compliquées.
- **Affichage Groupé et Coloré :** Les résultats sont regroupés par répertoire et mis en évidence avec des couleurs pour une meilleure lisibilité.

## Prérequis

Pour compiler et utiliser `fsearch`, vous aurez besoin de :
- **Windows** (le programme utilise l'API Win32)
- **CMake** (version 3.10 ou supérieure)
- Un **compilateur C++** (par exemple, celui fourni avec Visual Studio "Build Tools for Visual Studio")

## Comment Compiler

La méthode la plus simple est d'utiliser le script fourni.

### Méthode 1 : Script `build.bat`

1.  Assurez-vous que les commandes `cmake` et du compilateur sont accessibles dans votre terminal.
2.  Double-cliquez sur le fichier `build.bat` à la racine du projet.

Le script va automatiquement créer un répertoire `build`, générer les fichiers de projet et compiler l'exécutable.

### Méthode 2 : Manuellement

Si vous préférez compiler manuellement, ouvrez un terminal et exécutez les commandes suivantes depuis la racine du projet :

```powershell
# 1. Créer un répertoire pour la compilation
mkdir build

# 2. Se déplacer dans le répertoire de build
cd build

# 3. Générer les fichiers de projet avec CMake
cmake ..

# 4. Lancer la compilation
cmake --build .
```

Dans les deux cas, l'exécutable `fsearch.exe` sera généré dans le répertoire `build\Debug` (ou `build\Release` selon votre configuration).

## Utilisation

Une fois compilé, vous pouvez lancer une recherche depuis le terminal.

### Syntaxe

```
fsearch.exe "nom_du_fichier"
```
- Remplacez `"nom_du_fichier"` par le nom (ou une partie du nom) du fichier que vous recherchez.
- Les guillemets sont importants si le nom contient des espaces.

### Exemple

Pour rechercher tous les fichiers contenant "main.cpp" dans leur nom :

```powershell
# Si vous êtes dans le répertoire 'build'
.\Debug\fsearch.exe "main.cpp"

# Si vous êtes à la racine du projet
.\build\Debug\fsearch.exe "main.cpp"
```

### Exemple de Sortie

La sortie sera formatée pour être facile à lire :

```
Scanning... |
Scan complete!

--- Search Results ---

C:\Users\VotreNom\Desktop\saas\fsearch\build\CMakeFiles\3.23.2\CompilerIdCXX\
  -> main.cpp

C:\Users\VotreNom\Desktop\saas\fsearch\src\
  -> main.cpp

--- Summary ---
Found 2 file(s) in 2 directorie(s).
Time elapsed: 0.0872454 seconds.
```
