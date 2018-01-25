* Strong typedef for EntityID, ComponentID, TextureWrap, TextureFilter, etc
* Move entity/component stuff into its own namespaces like systems?
* Change from SFINAE to static asserts for better error messages for systems?
* Anisotropic filtering
* Add a tag system that doesnt require storage allocation (it would have to use the same component ids just not create the arrays)
* Make addComponent emplace objects?
* Move check for passing MAX_SYSTEM/COMPONENT into the get id function
* Change the check opengl error to use a macro so it audo disables when not in debug mode