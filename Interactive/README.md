
# Interactive


## Attempt 1

During attempt 1 of using the template as it, after changing necessary compiler paths and settings, the dog.gltf was able to load and textures worked. I first went about implementing a moveable camera so when I added more objects I could view them easier. After watching multiple videos of different types of cameras I settled on a FPS type camera controlled by wsad and the mouse.

The next objective was to get more than 1 asset loading and rendering, the first attempt was to use the content loader multiple times, however that overwrote the previous file meaning that a single content loading would not work in the context of loading multiple assets into the content loader. Whent trying to render the objects from the content loader only the last loaded would render.

## Attempt 2

Attempt 2 was using a vector of content loaders to load multiple assets into the scene, using the c++ vector<Content> and creating a new Content loader for every asset was seeming to work, it would load all assets and it would render them, however the textures would not use the correct texture for the models and would only use the last imported texture for all models.

## Attempt 3

Attempt 3 was almost identicle to attempt 2, the only difference was that when drawing each model I would bind the texture from each model specifically before rendering then delete the newely created texture as I found that when I didn't delete the new texture it would continually create new textures which would use more and more memory and in one event my program exited due to using all my computers memory. This new realisation that I could bind the texture before drawing the asset was key to getting the entire project working. 

## Attempt 4

Attempt 4 was mainly a refactor of all code I had implemented before, I created a struct to hold each asset in the scene, and by doing so I would only have to load the gltf files a single time instead of for every context as I had been doing before, due to this new struct I would only have to reference the context loader for that asset. The new struct held all information about the object, the "type", "name", "position", "offset", "rotation", "scale", "animation", "velocity".
  
- type: string - The name of the asset the object is using.
- name: string - The name of the object in the scene.
- position: vec3 - The position of the object in world space.
- offset: vec3 - The position of the object in local space
- rotation: vec3 - The rotation of the object.
- scale: vec3 - The scale of the object.
- animation: bool - Whether the object has an animation.
- velocity: vec3 - The velocity of the object.

The final structure of the code is there are multiple maps one which holds a string -> context and another which holds a string -> vector<Object>. The context map is accessably by using the type of each object so you only need to loop through the object loop every frame.

