// Game
#include <Game/World.hpp>
#include <Game/systems/ActionSystem.hpp>
#include <Game/systems/NetworkingSystem.hpp>
#include <Game/comps/ActionComponent.hpp>
#include <Game/comps/NetworkComponent.hpp>
#include <Game/comps/NetworkStatsComponent.hpp>
#include <Game/comps/PhysicsBodyComponent.hpp>


namespace {
	using namespace Game;
	using EstBuffSize = uint8;
	using Engine::ECS::Entity;
	using Engine::Net::MessageHeader;
	using Engine::Net::BufferReader;
	constexpr int32 tmax = std::numeric_limits<EstBuffSize>::max();
	constexpr int32 range = (tmax - ActionComponent::maxStates) / 2;

	constexpr EstBuffSize estBuffSizeToNet(const float32 v) noexcept {
		auto t = static_cast<int32>(v) + range;
		return std::max(0, std::min(t, tmax));
	};

	constexpr float32 estBuffSizeFromNet(EstBuffSize v) noexcept {
		return static_cast<float32>(v - range);
	};

	void recv_ACTION_server(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		auto& world = engine.getWorld();

		Engine::ECS::Tick tick;
		msg.read<Engine::ECS::Tick>(&tick);

		const auto recvTick = world.getTick();
		auto& actComp = world.getComponent<ActionComponent>(from.ent);

		// These are inclusive
		const auto minTick = recvTick + 1;
		const auto maxTick = recvTick + actComp.states.capacity() - 1 - 1; // Keep last input so we can duplicate if we need to

		for (auto t = tick + 1 - (actComp.states.capacity() / 4); t <= tick; ++t) {
			ActionState s = {};
			s.netRead(msg);

			if (t < minTick || t > maxTick) { continue; }
			if (!actComp.states.contains(t)) {
				if (t != tick) {
					//ENGINE_INFO("Oh boy. Back filled tick: ", t, " ", tick);
				} else {
					//ENGINE_WARN("Oh boy.", t);
				}

				s.recvTick = recvTick;
				actComp.states.insert(t) = s;
			}
		}
		msg.readFlushBits();

		{
			// TODO: why do we do this? isnt this always `tick - recvTick`?
			const float32 off = tick < recvTick ? -static_cast<float32>(recvTick - tick) : static_cast<float32>(tick - recvTick);
			// TODO: need to handle negative values. tick-recv vs recv-tick then cast to float and neg
			actComp.estBufferSize += (off - actComp.estBufferSize) * 0.2f;
		}

		//auto* found = actComp.states.find(tick);
		//
		//if (found) {
		//	ENGINE_WARN("Duplicate input for tick ", tick, ". Ignoring.");
		//	return;
		//} else {
		//	found = &actComp.states.insert(tick);
		//}
		//
		//*found = *state;
		//found->recvTick = recvTick;
	}

	void recv_ACTION_client(EngineInstance& engine, ConnectionInfo& from, const MessageHeader head, BufferReader& msg) {
		auto& world = engine.getWorld();
		constexpr float32 maxStates = static_cast<float32>(decltype(ActionComponent::states)::capacity());

		Engine::ECS::Tick tick;
		msg.read<Engine::ECS::Tick>(&tick);

		Engine::ECS::Tick recvTick;
		msg.read<Engine::ECS::Tick>(&recvTick);

		const auto buffSize = tick - recvTick;

		//
		//
		//
		// TODO: Zero is a valid tick. Fix.
		//
		//
		//

		// TODO (donfHIq7): The server should probably send a bitset of the recvs it has
		//                  received each time so it can be redundant like the client
		//                  side inputs are.
		if (recvTick == 0) {
			auto& actComp = world.getComponent<ActionComponent>(from.ent);
			auto* state = actComp.states.find(tick);
			if (state) {
				auto* prev = actComp.states.find(tick - 1);
				if (prev) {
					*state = *prev;
					ENGINE_WARN("Server missed input for ", tick, " - duplicating last input");
				} else {
					ENGINE_WARN("Server missed input for ", tick, " - unable to duplicate last input");
				}
			}
		}

		auto& actComp = world.getComponent<ActionComponent>(from.ent);
		{
			uint8 estNet;
			msg.read<uint8>(&estNet);
			const auto est = estBuffSizeFromNet(estNet);

			// NOTE: seems to work fine without this.
			//float32 ping = std::chrono::duration<float32, std::milli>{from.getPing()}.count();
			//float32 ticksPerPing = Engine::Clock::Milliseconds{from.getPing()} / Engine::Clock::Milliseconds{World::getTickInterval()};
			//
			//// Interpolate between 0.2 and 0.01 based on ping
			//constexpr float32 m = (0.01f - 0.2f) / (maxStates - 1.0f);
			//constexpr float32 b = (-maxStates * m) + 0.01f;
			//float32 smoothing = m * ticksPerPing + b;

			//actComp.estBufferSize += (est - actComp.estBufferSize) * smoothing;
			actComp.estBufferSize = est;
		}

		// TODO: this is ideal buffer size not tick lead...
		const auto idealTickLead = [&]{
			constexpr auto tickDur = World::getTickInterval();
			const auto p2 = from.getPing().count() * 0.5f; // One way trip
			const auto j2 = static_cast<float32>(from.getJitter().count());
			const auto avgTripTime = p2 * (1.0f + from.getLoss()) + j2;
			const auto avgTicksPerTrip = std::ceil(avgTripTime / tickDur.count());
			// TODO: probably also want to track a stat of how many inputs we have missed in the last X seconds. (exp avg would be fine)
			return avgTicksPerTrip + 1;
		};

		// TODO: ideal should be smoothed over time
		const auto ideal = idealTickLead();

		// Used to adjust tickScale when based on trend. Scaled in range [trendAdjust, maxTrendScale + trendAdjust]
		constexpr float32 maxTickScale = 2.0f;

		float32 diff = actComp.estBufferSize - ideal;

		const auto calcTickScale = [&](const float32 diff){
			float32 scale = (maxTickScale/(range + maxStates));
			const auto ping = from.getPing();

			// Be more conservative at larger pings to prevent bouncing between too high and too low due to slower feedback
			if (ping > std::chrono::milliseconds{350}) {
				scale *= 0.25f;
			} else if (ping > std::chrono::milliseconds{250}) {
				scale *= 0.50f;
			} else if (ping > std::chrono::milliseconds{100}) {
				scale *= 0.75f;
			} else if (ping > std::chrono::milliseconds{50}) {
				scale *= 0.95f;
			}

			return 1.0f + (diff * scale);
		};

		// NOTE: seems to work fine without this
		// Prevent large changes from ping spikes
		//if (from.getJitter() > std::chrono::milliseconds{50}) {
		//	diff *= 0.25f;
		//}

		// Slow down when we are close to the correct value
		if (std::abs(diff) < ideal + 8.0f) {
			diff *= 0.75f;
		}

		// Determine the tick scale
		constexpr float32 eps = 0.8f;
		if (diff < -eps) {
			world.tickScale = 1.0f / calcTickScale(std::abs(diff));
		} else if (diff > eps) {
			world.tickScale = calcTickScale(diff);
		} else {
			world.tickScale = 1.0f;
		}

		if constexpr (ENGINE_DEBUG) {
			if (world.hasComponent<NetworkStatsComponent>(from.ent)) {
				auto& netStatsComp = world.getComponent<NetworkStatsComponent>(from.ent);
				netStatsComp.inputBufferSize = static_cast<int32>(buffSize);
				netStatsComp.idealInputBufferSize = ideal;
			}
		}
	}
}


namespace Game {
	ActionSystem::ActionSystem(SystemArg arg)
		: System{arg} {
	}

	void ActionSystem::setup() {
		auto& netSys = world.getSystem<NetworkingSystem>();
		netSys.setMessageHandler(MessageType::ACTION, ENGINE_SERVER ? recv_ACTION_server : recv_ACTION_client);
	}

	void ActionSystem::preTick() {
		for (const auto ent : world.getFilter<ActionComponent>()) {
			const auto tick = world.getTick();
			auto& actComp = world.getComponent<ActionComponent>(ent);

			if (ENGINE_SERVER || world.isPerformingRollback()) {
				actComp.state = actComp.states.find(tick);
			} else {
				const auto& last = actComp.states.get(tick - 1);
				actComp.state = &actComp.states.insert(tick);
				*actComp.state = last;
				for (auto& btnState : actComp.state->buttons) {
					btnState.pressCount = 0;
					btnState.releaseCount = 0;
				}
			}
		}
	}

	void ActionSystem::tick() {
		if (world.isPerformingRollback()) { return; }

		auto& cam = engine.getCamera();
		const auto currTick = world.getTick();

		// TODO (donfHIq7): The server side should be in System::network instead. Not
		//                  worth fussing with at the moment though The client side
		//                  should remain in update so the server has the most
		//                  up-to-date inputs and reduced buffer size.

		for (const auto ent : world.getFilter<ActionComponent, NetworkComponent>()) {
			auto& actComp = world.getComponent<ActionComponent>(ent);

			// TODO: this should probably be moved, this doesnt really depend on the connection status/object/etc
			if constexpr (ENGINE_CLIENT) {
				// TODO: update cursor axis locations
				const auto& physComp = world.getComponent<PhysicsBodyComponent>(ent);
				const auto& pos = physComp.getPosition();
				auto& state = *actComp.state;
				const auto& tpos = cam.screenToWorld(state.screenTarget);
				state.target.x = tpos.x - pos.x;
				state.target.y = tpos.y - pos.y;
			}

			auto& conn = world.getComponent<NetworkComponent>(ent).get();

			ENGINE_DEBUG_ONLY(conn._debug_AllowMessages = true);

			if constexpr (ENGINE_CLIENT) {
				// Hey! are you wondering why the client sends so much data again?
				// Well let me save you some time. This code sends about
				// 12288 bytes per second assuming 64 tick and 64 action history states (sending 1/4 of that).
				// If we want to do better we need to compress this or send fewer states.
				// Check trello for more complete explanation. See: O3oJLMde

				if (auto msg = conn.beginMessage<MessageType::ACTION>()) {
					msg.write(currTick);

					// TODO: how many to send?
					for (auto t = currTick + 1 - (actComp.states.capacity() / 4); t <= currTick; ++t) {
						const auto& s = actComp.states.get(t);
						for (auto b : s.buttons) {
							msg.write<2>(b.pressCount);
							msg.write<2>(b.releaseCount);
							msg.write<1>(b.latest);
						}

						// TODO: compress. we dont need 32 bits here.
						// TODO: if you compress this make sure to replicate on client to remain in sync
						msg.write<32>(reinterpret_cast<const uint32&>(s.target.x));
						msg.write<32>(reinterpret_cast<const uint32&>(s.target.y));
					}

					msg.writeFlushBits();
				}
			} else if constexpr (ENGINE_SERVER) {
				auto* state = actComp.state;
				
				// Send action ACK and est. buffer update.
				if (auto msg = conn.beginMessage<MessageType::ACTION>()) {
					msg.write(currTick);
					msg.write(state ? state->recvTick : 0);
					msg.write(estBuffSizeToNet(actComp.estBufferSize));
				}

				// Insert missing states.
				if (!state) {
					// TODO: duplicate and decay last input?
					state = actComp.states.find(currTick - 1);
					actComp.state = &(actComp.states.insert(currTick) = (state ? *state : ActionState{}));
					ENGINE_LOG("Missing input for tick ", currTick, state ? " - using previous input" : " - unable to duplicate previous input"); // TODO: message about duplicate or zero
				}

				// Update network stats.
				if constexpr (ENGINE_DEBUG) {
					if (world.hasComponent<NetworkStatsComponent>(ent)) {
						auto& netStatsComp = world.getComponent<NetworkStatsComponent>(ent);
						// This should be `ceil(estBuffSize) + 1` the + 1 since we also keep last tick for duplication
						netStatsComp.inputBufferSize = actComp.states.max() - currTick + 2; // max - (cur - 1) + 1 = max - cur + 2
					}
				}

				//ENGINE_LOG("Buffer size: ",
				//	actComp.states.capacity(), " | ",
				//	actComp.states.max(), " ", actComp.states.minValid(), " = ",
				//	actComp.states.max() - actComp.states.minValid());
				// If we ever add lag compensation we will need to handle server rollback here.
			}

			ENGINE_DEBUG_ONLY(conn._debug_AllowMessages = false);
		}
	}

	void ActionSystem::updateActionState(Action act, bool val) {
		for (const auto& ent : world.getFilter<ActionComponent>()) {
			updateActionState(ent, act, val);
		}
	}

	void ActionSystem::updateActionState(Engine::ECS::Entity ent, Action act, bool val) {
		auto& actComp = world.getComponent<ActionComponent>(ent);
		auto& value = actComp.state->buttons[static_cast<int32>(act)];
		value.pressCount += val;
		value.releaseCount += !val;
		value.latest = val;
	}
	
	void ActionSystem::updateTarget(glm::vec2 val) {
		for (const auto& ent : world.getFilter<ActionComponent>()) {
			updateTarget(ent, val);
		}
	}

	void ActionSystem::updateTarget(Engine::ECS::Entity ent, glm::vec2 val) {
		auto& actComp = world.getComponent<ActionComponent>(ent);
		actComp.state->screenTarget = val;
	}
}
