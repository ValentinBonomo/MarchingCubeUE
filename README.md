# Terrain Destructible – Projet Unreal Engine 5.6.1

Ce projet implémente un **système de terrain destructible basé sur des voxels**, généré à l’aide de **Marching Cubes** et modifiable en temps réel par le joueur.  
Le terrain est découpé en chunks, généré via Perlin Noise (*FastNoiseLite*), et possède un système de sauvegarde permettant de conserver les modifications du monde.

---

## 🎮 Fonctionnalités principales

- Génération procédurale du terrain (voxels + Marching Cubes)
- Édition du terrain en temps réel (creuser / ajouter)
- Affichage d’un **brush sphérique** pour visualiser la zone modifiée
- Système de **chunks** dynamiques autour du joueur
- Système de **sauvegarde / chargement** des chunks modifiés
- Réinitialisation complète du monde

---

## 🎮 Contrôles

| Action | Commande |
|--------|----------|
| Déplacement du joueur | **W A S D** |
| Creuser (enlever du terrain) | **Clic gauche** |
| Ajouter du terrain | **Clic droit** |
| Réinitialiser la sauvegarde (après redémarrage du jeu)** | **R** |

**Note :** La touche **R** supprime la sauvegarde existante. Le reset est appliqué lorsque le jeu est relancé.

---

## 🧱 Édition du terrain

L’édition se fait via un brush sphérique visible sous la souris :

- Le brush suit le point d’impact du curseur sur le terrain.
- Le rayon et la force sont configurables dans les paramètres de l’acteur `VoxelWorld`.
- Les modifications affectent automatiquement tous les chunks voisins touchés par le rayon.

---

## 💾 Sauvegarde

- Chaque chunk modifié est enregistré avec :  
  - sa position dans le monde  
  - ses densités de voxels
- Les données sont stockées dans un `SaveGame` (`VoxelWorldSave`)
- Au lancement du jeu :
  - les chunks sauvegardés sont restaurés
  - les autres sont générés à partir du bruit

---

## 🔧 Configuration dans l’éditeur

Sélectionner l’acteur **VoxelWorld** dans la scène :

- **Chunk Size** : taille d’un chunk (en voxels)
- **Voxel Size** : taille d’un voxel (en unités UE)
- **Render Distance** : nombre de chunks autour du joueur
- **Brush Radius** : rayon du brush
- **Brush Strength** : force appliquée lors de l’édition  
