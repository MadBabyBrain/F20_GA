
# Animation

From the world_V4 render file each group of objects was exported. Some objects were removed as duplicated such as the claws, machines, trees and grass. Most items were imported easily, however claws and lamps had a couple of problems:

The claws had their transforms all wrong but only needed their transforms applied when exporting them to .fbx files, the lamps were fine apart from their entire base was indisde out, which required going back to blender and flipping the faces of the pole so that the outside was visible again.

The animations were all created in blender using keyframes and some precise angles and then matching these precise angles, the claws were animated in an arc motion and the cubes followed along with them, the cubes only needed to be animated moving across the belts and then following the claws.

The camera animation was created in unreal using theire sequencer and having the camera follow a spline to navigate the environment.

## Output

![Video](https://user-images.githubusercontent.com/46905566/201343025-a954c015-141e-4fe3-8fb1-97d18c38c2c1.mp4)
