#pragma once


namespace Game {
	class World; // Forward declaration

	/**
	 * A base class from systems.
	 */
	class SystemBase {
		protected:
			/** The world that owns this system. */
			World& world;

		public:
			/**
			 * Constructor.
			 * @param[in,out] world The world that owns this system.
			 */
			SystemBase(World& world) : world{world} {};
	};
}
