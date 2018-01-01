* Strong typedef for EntityID, ComponentID, TextureWrap, TextureFilter, etc
* Consider adding a priority/dependency system to systems
* Move entity/component stuff into its own namespaces like systems?
* Change from SFINAE to static asserts for better error messages for systems?
* Anisotropic filtering
* Add a tag system that doesnt require storage allocation (it would have to use the same component ids just not create the arrays)
* Make addComponent emplace objects?
* Move check for passing MAX_SYSTEM/COMPONENT into the get id function
* Update IsSystem for priorities
* Change the ECS to use classes not globals.