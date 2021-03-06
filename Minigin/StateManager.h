#pragma once
#include "State.h"
#include <unordered_map>
#include <iostream>
#include <vector>
#include <windows.h>
#include "Logger.h"

using namespace std;

namespace minigin
{
	struct EnumClassHash final
	{
		template <typename T>
		size_t operator()(T t) const
		{
			return static_cast<size_t>(t);
		}
	};

	template<typename states>
	class StateManager
	{
	public:
		//rule of 5
		StateManager() = default;
		virtual ~StateManager() = default;

		StateManager(const StateManager& other) = delete;
		StateManager(StateManager&& other) noexcept = delete;
		StateManager& operator=(const StateManager& other) = delete;
		StateManager& operator=(StateManager&& other) noexcept = delete;

		//non virtual they aren't supposed to be edited in derived
		void Update();
		void Render() const;

		void AddState(states state, State<states>* pStateClass, bool setCurrentState = false);

	protected:
		//datamembers
		unordered_map<states, State<states>*, EnumClassHash> m_States;
		pair<states, State<states>*> m_pActiveState;

		//functions
		void ChangeState(states newState);
	};

	template<typename states>
	void StateManager<states>::Update()
	{
		states newState{ m_pActiveState.second->Update() };

		if (newState != m_pActiveState.first) ChangeState(newState);
	}

	template<typename states>
	void StateManager<states>::Render() const
	{
		m_pActiveState.second->Render();
	}

	template<typename states>
	void StateManager<states>::AddState(states state, State<states>* pStateClass, bool setCurrentState)
	{
		auto pair{ std::pair<states, State<states>*>(state, pStateClass) };
		m_States.insert(pair);

		if (setCurrentState)
		{
			m_pActiveState = pair;
			m_pActiveState.second->StartAction();
		}
	}

	template<typename states>
	void StateManager<states>::ChangeState(states newState)
	{
		m_pActiveState.second->EndAction();

		auto it = m_States.find(newState);

		if (it == m_States.cend()) Logger::LogError(L"Invalid state requested!");
		else
		{
			m_pActiveState = *it;
			m_pActiveState.second->StartAction();
		}
	}
}


