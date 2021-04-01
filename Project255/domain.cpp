#include "domain.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

inline std::string_view sub_string(std::string_view s, std::size_t p, std::size_t n) {
	return s.substr(p, n);
}

void ltrim(std::string& s) {
	s.erase(
		s.begin(),
		std::find_if(
			s.begin(),
			s.end(),
			[](unsigned char ch) {
				return !std::isspace(ch);
			}
		)
	);
}

void rtrim(std::string& s) {
	s.erase(
		std::find_if(
			s.rbegin(),
			s.rend(),
			[](unsigned char ch) {
				return !std::isspace(ch);
			}
		).base(),
		s.end()
	);
}

void mtrim(std::string& s) {
	std::string::iterator it = std::unique(s.begin(), s.end(), [](auto lhs, auto rhs) { return lhs == rhs && lhs == ' '; });
	s.erase(it, s.end());
}

void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}

void ftrim(std::string& s) {
	ltrim(s);
	rtrim(s);
	mtrim(s);
}

InputRequestType UserInputData::getRequestType() const {
	return _request_type;
}

void UserInputData::setRequestType(InputRequestType rt) {
	_request_type = rt;
}

RouteStopInputData::RouteStopInputData(std::string stop_name, Coordinates coordinates, Distances distances) : _stop_name(std::move(stop_name)), _coordinates(coordinates), _distances(std::move(distances)) {
	setRequestType(InputRequestType::RouteStop);
}

std::string& RouteStopInputData::getStopName() {
	return _stop_name;
}

Coordinates RouteStopInputData::getCoordinates() {
	return _coordinates;
}

Distances& RouteStopInputData::getDistances() {
	return _distances;
}

BusInputData::BusInputData(std::vector<std::string> stop_names, BusID bus_id, bool isCircle) : _stop_names(std::move(stop_names)), _bus_id(std::move(bus_id)), _isCircle(isCircle) {
	setRequestType(InputRequestType::Bus);
}

std::vector<std::string>& BusInputData::getStopNames() {
	return _stop_names;
}

BusID& BusInputData::getBusID() {
	return _bus_id;
}

bool BusInputData::getIsCircle() {
	return _isCircle;
}

UserStatData::UserStatData(int id) : m_id(id) {}

StatRequestType UserStatData::getRequestType() const {
	return _request_type;
}

int UserStatData::getRequestID() const {
	return m_id;
}

void UserStatData::setRequestType(StatRequestType rt) {
	_request_type = rt;
}

BusStatInputData::BusStatInputData(int id, BusID bus_id) : UserStatData(id), _bus_id(std::move(bus_id)) {
	setRequestType(StatRequestType::BusStat);
}

BusID& BusStatInputData::getBusID() {
	return _bus_id;
}

StopStatInputData::StopStatInputData(int id, std::string stop_name) : UserStatData(id), _stop_name(stop_name) {
	setRequestType(StatRequestType::StopStat);
}

std::string& StopStatInputData::getStopName() {
	return _stop_name;
}

const std::string EvtData_Before_Start_Processing::sk_EventName = "EvtData_Before_Start_Processing";

EvtData_Before_Start_Processing::EvtData_Before_Start_Processing() : m_out(std::cout) {}

EvtData_Before_Start_Processing::EvtData_Before_Start_Processing(std::ostream& out) : m_out(out) {}

const EventTypeId& EvtData_Before_Start_Processing::VGetEventType() const {
	return sk_EventType;
}

const std::string& EvtData_Before_Start_Processing::GetName() const {
	return sk_EventName;
}

std::ostream& EvtData_Before_Start_Processing::GetOutput() {
	return m_out;
}

std::ostream& operator<<(std::ostream& os, const EvtData_Before_Start_Processing& evt) {
	std::ios::fmtflags oldFlag = os.flags();
	os << "Event type id: " << evt.sk_EventType << std::endl;
	os << "Event name: " << evt.sk_EventName << std::endl;
	os.flags(oldFlag);
	return os;
}

const std::string EvtData_Before_User_Data_Processing::sk_EventName = "EvtData_Before_User_Data_Processing";

EvtData_Before_User_Data_Processing::EvtData_Before_User_Data_Processing() : m_last_entry(false), m_first_entry(false), m_out(std::cout) {}

EvtData_Before_User_Data_Processing::EvtData_Before_User_Data_Processing(std::ostream& out, bool last_entry, bool first_entry) : m_last_entry(last_entry), m_first_entry(first_entry), m_out(out) {}

const EventTypeId& EvtData_Before_User_Data_Processing::VGetEventType() const {
	return sk_EventType;
}

const std::string& EvtData_Before_User_Data_Processing::GetName() const {
	return sk_EventName;
}

std::ostream& EvtData_Before_User_Data_Processing::GetOutput() {
	return m_out;
}

bool EvtData_Before_User_Data_Processing::IslastEntry() {
	return m_last_entry;
}

bool EvtData_Before_User_Data_Processing::IsFirstEntry() {
	return m_first_entry;
}

std::ostream& operator<<(std::ostream& os, const EvtData_Before_User_Data_Processing& evt) {
	std::ios::fmtflags oldFlag = os.flags();
	os << "Event type id: " << evt.sk_EventType << std::endl;
	os << "Event name: " << evt.sk_EventName << std::endl;
	os.flags(oldFlag);
	return os;
}

const std::string EvtData_After_User_Data_Processing::sk_EventName = "EvtData_After_User_Data_Processing";

EvtData_After_User_Data_Processing::EvtData_After_User_Data_Processing() : m_last_entry(false), m_first_entry(false), m_out(std::cout) {}

EvtData_After_User_Data_Processing::EvtData_After_User_Data_Processing(std::ostream& out, bool last_entry, bool first_entry) : m_last_entry(last_entry), m_first_entry(first_entry), m_out(out) {}

const EventTypeId& EvtData_After_User_Data_Processing::VGetEventType() const {
	return sk_EventType;
}

const std::string& EvtData_After_User_Data_Processing::GetName() const {
	return sk_EventName;
}

std::ostream& EvtData_After_User_Data_Processing::GetOutput() {
	return m_out;
}

bool EvtData_After_User_Data_Processing::IslastEntry() {
	return m_last_entry;
}

bool EvtData_After_User_Data_Processing::IsFirstEntry() {
	return m_first_entry;
}

std::ostream& operator<<(std::ostream& os, const EvtData_After_User_Data_Processing& evt) {
	std::ios::fmtflags oldFlag = os.flags();
	os << "Event type id: " << evt.sk_EventType << std::endl;
	os << "Event name: " << evt.sk_EventName << std::endl;
	os.flags(oldFlag);
	return os;
}

const std::string EvtData_After_End_Processing::sk_EventName = "EvtData_After_End_Processing";

EvtData_After_End_Processing::EvtData_After_End_Processing() : m_out(std::cout) {}

EvtData_After_End_Processing::EvtData_After_End_Processing(std::ostream& out) : m_out(out) {}

const EventTypeId& EvtData_After_End_Processing::VGetEventType() const {
	return sk_EventType;
}

const std::string& EvtData_After_End_Processing::GetName() const {
	return sk_EventName;
}

std::ostream& EvtData_After_End_Processing::GetOutput() {
	return m_out;
}

std::ostream& operator<<(std::ostream& os, const EvtData_After_End_Processing& evt) {
	std::ios::fmtflags oldFlag = os.flags();
	os << "Event type id: " << evt.sk_EventType << std::endl;
	os << "Event name: " << evt.sk_EventName << std::endl;
	os.flags(oldFlag);
	return os;
}

IEventManager* g_pEventMgr = NULL;
GenericObjectFactory<IEventData, EventTypeId> g_eventFactory;

IEventManager* IEventManager::Get() {
	return g_pEventMgr;
}

IEventDataPtr IEventManager::Create(EventTypeId eventType) {
	return IEventDataPtr(CREATE_EVENT(eventType));
}

IEventManager::IEventManager(bool setAsGlobal) {
	if (setAsGlobal) {
		if (g_pEventMgr) {
			delete g_pEventMgr;
		}

		g_pEventMgr = this;
	}
}

IEventManager::~IEventManager() {
	if (g_pEventMgr == this)
		g_pEventMgr = NULL;
}

EventManager::EventManager(const std::string& pName, bool setAsGlobal) : IEventManager(setAsGlobal), m_eventManagerName(pName) {
	m_activeQueue = 0;
}

bool EventManager::VAddListener(const EventListenerDelegate& eventDelegate, const EventTypeId& type) {
	auto& eventListenerList = m_eventListeners[type];
	for (auto it = eventListenerList.begin(); it != eventListenerList.end(); ++it) {
		if (eventDelegate == (*it)) {
			return false;
		}
	}
	eventListenerList.push_back(eventDelegate);
	return true;
}

bool EventManager::VRemoveListener(const EventListenerDelegate& eventDelegate, const EventTypeId& type) {
	bool success = false;
	auto findIt = m_eventListeners.find(type);
	if (findIt != m_eventListeners.end()) {
		auto& listeners = findIt->second;
		for (auto it = listeners.begin(); it != listeners.end(); ++it) {
			if (eventDelegate == (*it)) {
				listeners.erase(it);
				success = true;
				break;
			}
		}
	}
	return success;
}

bool EventManager::VTriggerEvent(const IEventDataPtr& pEvent) const {
	bool processed = false;
	auto findIt = m_eventListeners.find(pEvent->VGetEventType());
	if (findIt != m_eventListeners.end()) {
		const auto& eventListenerList = findIt->second;
		for (auto it = eventListenerList.begin(); it != eventListenerList.end(); ++it) {
			auto listener = (*it);
			listener(pEvent);
			processed = true;
		}
	}
	return processed;
}

bool EventManager::VQueueEvent(const IEventDataPtr& pEvent) {
	// make sure the event is valid
	if (!pEvent) {
		return false;
	}

	auto findIt = m_eventListeners.find(pEvent->VGetEventType());
	if (findIt != m_eventListeners.end()) {
		m_queues[m_activeQueue].push_back(pEvent);
		return true;
	}
	else {
		return false;
	}
}

bool EventManager::VUpdate() {
	int queueToProcess = m_activeQueue;
	m_activeQueue = (m_activeQueue + 1) % EVENTMANAGER_NUM_QUEUES;
	m_queues[m_activeQueue].clear();

	while (!m_queues[queueToProcess].empty()) {
		auto pEvent = m_queues[queueToProcess].front();
		m_queues[queueToProcess].pop_front();

		const unsigned long& eventType = pEvent->VGetEventType();

		auto findIt = m_eventListeners.find(eventType);
		if (findIt != m_eventListeners.end()) {
			const auto& eventListeners = findIt->second;

			for (auto it = eventListeners.begin(); it != eventListeners.end(); ++it) {
				auto listener = (*it);
				listener(pEvent);
			}
		}
	}

	bool queueFlushed = (m_queues[queueToProcess].empty());
	if (!queueFlushed) {
		while (!m_queues[queueToProcess].empty()) {
			auto pEvent = m_queues[queueToProcess].back();
			m_queues[queueToProcess].pop_back();
			m_queues[m_activeQueue].push_front(pEvent);
		}
	}

	return queueFlushed;
}

bool EventManager::VAbortEvent(const EventTypeId& inType, bool allOfType) {
	bool success = false;
	auto findIt = m_eventListeners.find(inType);

	if (findIt != m_eventListeners.end()) {
		auto& eventQueue = m_queues[m_activeQueue];
		auto it = eventQueue.begin();
		while (it != eventQueue.end()) {
			auto thisIt = it;
			++it;

			if ((*thisIt)->VGetEventType() == inType) {
				eventQueue.erase(thisIt);
				success = true;
				if (!allOfType)
					break;
			}
		}
	}

	return success;
}

std::ostream& operator<<(std::ostream& os, const EventManager& mgr) {
	std::ios::fmtflags oldFlag = os.flags();

	std::cout << "EventManager name: " << mgr.m_eventManagerName << std::endl;
	std::cout << "Contains listeners:" << std::endl;
	int counter = 0;
	for (const auto& [eventTypeId, listenerFx] : mgr.m_eventListeners) {
		std::cout << ++counter << ") Listener for event type id: " << eventTypeId << " with name: " << GET_EVENT_NAME(eventTypeId) << std::endl;
	}
	std::cout << "Current active queue: " << mgr.m_activeQueue << std::endl;
	std::cout << "Events queue contains:" << std::endl;
	int queueCounter = 0;
	int eventCounter = 0;
	for (const auto& currentQueue : mgr.m_queues) {
		std::cout << queueCounter++ << ") queue ->" << std::endl;
		for (const auto& currentEvent : currentQueue) {
			std::cout << "\t" << ++eventCounter << ") event id: " << currentEvent->VGetEventType() << " with name: " << currentEvent->GetName() << std::endl;
		}
	}

	os.flags(oldFlag);
	return os;
}

MapStatInputData::MapStatInputData(int id, const RenderSettings& settings) : UserStatData(id), _settings(settings) {
	setRequestType(StatRequestType::Map);
}

const RenderSettings& MapStatInputData::getRenderSettings() {
	return _settings;
}
