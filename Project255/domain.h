#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>
#include <string>
#include <regex>
#include <algorithm>
#include <iterator>
#include <limits>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <sstream>
#include <set>
#include <iostream>

#include "svg.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

using namespace std::literals;

template <typename T_SRC, typename T_DEST, typename T_DELETER>
bool dynamic_pointer_move(std::unique_ptr<T_DEST, T_DELETER>& dest,	std::unique_ptr<T_SRC, T_DELETER>& src) {
	if (!src) {
		dest.reset();
		return true;
	}

	T_DEST* dest_ptr = dynamic_cast<T_DEST*>(src.get());
	if (!dest_ptr) {
		return false;
	}

	std::unique_ptr<T_DEST, T_DELETER> dest_temp(dest_ptr, std::move(src.get_deleter()));

	src.release();
	dest.swap(dest_temp);
	return true;
}

template <typename T_SRC, typename T_DEST>
bool dynamic_pointer_move(std::unique_ptr<T_DEST>& dest,
	std::unique_ptr<T_SRC>& src) {
	if (!src) {
		dest.reset();
		return true;
	}

	T_DEST* dest_ptr = dynamic_cast<T_DEST*>(src.get());
	if (!dest_ptr)
		return false;

	src.release();
	dest.reset(dest_ptr);
	return true;
}

template<typename C, typename T>
auto insert_in_container(C& c, T&& t) -> decltype(c.push_back(std::forward<T>(t)), void()) {
	c.push_back(std::forward<T>(t));
}

template<typename C, typename T>
auto insert_in_container(C& c, T&& t) -> decltype(c.insert(std::forward<T>(t)), void()) {
	c.insert(std::forward<T>(t));
}

template <typename Container>
auto its_and_idx(Container&& container) {
	return std::tuple{ std::begin(container), std::end(container), 0 };
}

inline std::string_view sub_string(std::string_view s, std::size_t p, std::size_t n = std::string_view::npos);

void ltrim(std::string& s);

void rtrim(std::string& s);

void mtrim(std::string& s);

void trim(std::string& s);

void ftrim(std::string& s);

template<typename Container>
class Splitter {
public:
	Splitter(std::string separatorVerb) : _separatorVerb(std::move(separatorVerb)) {};
	virtual Container Split(std::istream& in) = 0;
	virtual Container Split(std::string& in) = 0;
	virtual const std::string& getSeparatorVerb() { return _separatorVerb; }
protected:
	std::string _separatorVerb;
};

template<typename Container>
class RegExExtractor : public Splitter<Container> {
private:
	Container splitR(const std::string& input, const std::string& regex) {
		std::regex re(regex);
		std::sregex_token_iterator first(input.begin(), input.end(), re, 0);
		std::sregex_token_iterator last;
		return Container(first, last);
	}

	Container splitR(std::istream& in, const std::string& regex) {
		std::string line;
		std::getline(in, line);
		return splitR(line, regex);
	}

public:
	RegExExtractor(std::string separatorVerb) : Splitter<Container>(std::move(separatorVerb)) {};
	virtual Container Split(std::istream& in) override {
		return splitR(in, this->_separatorVerb);
	}
	virtual Container Split(std::string& in) override {
		return splitR(in, this->_separatorVerb);
	}
};

template<typename Container>
class StlSplitter : public Splitter<Container> {
private:
	Container splitR(const std::string& input, const std::string& delims) {
		Container out;
		size_t delims_len = delims.size();
		size_t begIdx = 0u;
		size_t endIdx = input.find(delims, begIdx);
		if (endIdx == std::string::npos && input.size() != 0u) {
			insert_in_container(out, input);
		}
		while (endIdx != std::string::npos) {
			insert_in_container(out, input.substr(begIdx, endIdx - begIdx));
			begIdx = endIdx + delims_len;
			endIdx = input.find(delims, begIdx);
			if (endIdx == std::string::npos) {
				insert_in_container(out, input.substr(begIdx, input.length() - begIdx));
			}
		}
		return out;
	}

	Container splitR(std::istream& in, const std::string& delims) {
		std::string line;
		std::getline(in, line);
		return splitR(line, delims);
	}

public:
	StlSplitter(std::string separatorVerb) : Splitter<Container>(std::move(separatorVerb)) {};
	virtual Container Split(std::istream& in) override {
		return splitR(in, this->_separatorVerb);
	}
	virtual Container Split(std::string& in) override {
		return splitR(in, this->_separatorVerb);
	}
};

template<typename Container>
class StlSplitterTrimmer : public Splitter<Container> {
private:
	Container splitR(const std::string& input, const std::string& delims) {
		Container out;
		auto begIdx = input.find_first_not_of(delims);
		while (begIdx != std::string::npos) {
			auto endIdx = input.find_first_of(delims, begIdx);
			if (endIdx == std::string::npos) {
				endIdx = input.length();
			}
			std::string tmp = input.substr(begIdx, endIdx - begIdx);
			ftrim(tmp);
			insert_in_container(out, tmp);
			begIdx = input.find_first_not_of(delims, endIdx);
		}
		return out;
	}

	Container splitR(std::istream& in, const std::string& delims) {
		std::string line;
		std::getline(in, line);
		return splitR(line, delims);
	}

public:
	StlSplitterTrimmer(std::string separatorVerb) : Splitter<Container>(std::move(separatorVerb)) {};
	virtual Container Split(std::istream& in) override {
		return splitR(in, this->_separatorVerb);
	}
	virtual Container Split(std::string& in) override {
		return splitR(in, this->_separatorVerb);
	}
};

struct Coordinates {
	double lat; // Широта
	double lng; // Долгота
};

enum class StreamType {
	TEXT,
	JSON,
	XML
};

using BusID = std::string;
using RouteStopName = std::string;
using RouteStopLocation = Coordinates;
using dist = int;
using Distances = std::unordered_map<std::string, dist>;

struct LocalBuses {
	RouteStopLocation location;
	std::set<BusID> buses;
	Distances distances;
};

struct Route {
	std::vector<RouteStopName> stops;
	bool isRouteCircle;
};

struct Trace {
	BusID bus_num;
	const Route* route;
};

using RouteStops = std::unordered_map<RouteStopName, LocalBuses>;
using Buses = std::unordered_map<BusID, Route>;

struct LocalBusFullRef {
	const Buses* all_buses;
	const RouteStops* route_stops;
};

struct LocalBusFull {
	RouteStopLocation location;
	RouteStopName name;
	std::set<BusID> buses;
};

struct FullRouteInfo {
	std::vector<LocalBusFull> stops;
	bool isRouteCircle;
};

using RoutesInfo = std::unordered_map<BusID, FullRouteInfo>;

enum class InputRequestType {
	RouteStop,
	Bus
};

class UserInputData {
public:
	InputRequestType getRequestType() const;
	virtual ~UserInputData() = default;
protected:
	void setRequestType(InputRequestType rt);
private:
	InputRequestType _request_type;
};

class RouteStopInputData : public UserInputData {
public:
	RouteStopInputData(std::string stop_name, Coordinates coordinates, Distances distances);
	std::string& getStopName();
	Coordinates getCoordinates();
	Distances& getDistances();

private:
	std::string _stop_name;
	Coordinates _coordinates;
	Distances _distances;
};

class BusInputData : public UserInputData {
public:
	BusInputData(std::vector<std::string> stop_names, BusID bus_id, bool isCircle);
	std::vector<std::string>& getStopNames();
	BusID& getBusID();
	bool getIsCircle();
private:
	std::vector<std::string> _stop_names;
	BusID _bus_id;
	bool _isCircle;
};

class InputReader {
public:
	virtual std::vector<std::unique_ptr<UserInputData>> getUserInput(std::istream& in) = 0;
	virtual ~InputReader() = default;
};

enum class StatRequestType {
	BusStat,
	StopStat,
	Map
};

class UserStatData {
public:
	UserStatData(int id);
	StatRequestType getRequestType() const;
	int getRequestID() const;
	virtual ~UserStatData() = default;
protected:
	void setRequestType(StatRequestType rt);
private:
	StatRequestType _request_type;
	int m_id;
};

class BusStatInputData : public UserStatData {
public:
	BusStatInputData(int id, BusID BusID);
	BusID& getBusID();

private:
	BusID _bus_id;
};

class StopStatInputData : public UserStatData {
public:
	StopStatInputData(int id, std::string stop_name);
	std::string& getStopName();

private:
	std::string _stop_name;
};

struct RenderSettings {
	double width;
	double height;
	double padding;
	double line_width;
	double stop_radius;

	uint32_t bus_label_font_size;
	svg::Point bus_label_offset;

	uint32_t stop_label_font_size;
	svg::Point stop_label_offset;

	svg::Color underlayer_color;
	double underlayer_width;

	std::vector<svg::Color> color_palette;
};

class MapStatInputData : public UserStatData {
public:
	MapStatInputData(int id, const RenderSettings& settings);
	const RenderSettings& getRenderSettings();

private:
	RenderSettings _settings;
};

class StatReader {
public:
	virtual std::vector<std::unique_ptr<UserStatData>> getUserStat(std::istream& in) = 0;
	virtual ~StatReader() = default;
};

class IOReader : public InputReader, public StatReader {};

template<typename... Type>
struct type_list {
    using type = type_list;
    static constexpr auto size = sizeof...(Type);
};

template<std::size_t, typename>
struct type_list_element;

template<std::size_t Index, typename Type, typename... Other>
struct type_list_element<Index, type_list<Type, Other...>> : type_list_element<Index - 1u, type_list<Other...>> {};

template<std::size_t Index, typename List>
using type_list_element_t = typename type_list_element<Index, List>::type;

template<typename To, typename From>
struct constness_as {
    using type = std::remove_const_t<To>;
};

template<typename To, typename From>
using constness_as_t = typename constness_as<To, From>::type;

namespace internal {
    template<typename Ret, typename... Args>
    auto function_pointer(Ret(*)(Args...))->Ret(*)(Args...);

    template<typename Ret, typename Type, typename... Args, typename Other>
    auto function_pointer(Ret(*)(Type, Args...), Other&&)->Ret(*)(Args...);

    template<typename Class, typename Ret, typename... Args, typename... Other>
    auto function_pointer(Ret(Class::*)(Args...), Other &&...)->Ret(*)(Args...);

    template<typename Class, typename Ret, typename... Args, typename... Other>
    auto function_pointer(Ret(Class::*)(Args...) const, Other &&...)->Ret(*)(Args...);

    template<typename Class, typename Type, typename... Other>
    auto function_pointer(Type Class::*, Other &&...)->Type(*)();

    template<typename... Type>
    using function_pointer_t = decltype(internal::function_pointer(std::declval<Type>()...));

    template<typename... Class, typename Ret, typename... Args>
    [[nodiscard]]
    constexpr auto index_sequence_for(Ret(*)(Args...)) {
        return std::index_sequence_for<Class..., Args...>{};
    }
}

template<auto>
struct connect_arg_t {};

template<auto Func>
inline constexpr connect_arg_t<Func> connect_arg{};

template<typename>
class delegate;

template<typename Ret, typename... Args>
class delegate<Ret(Args...)> {
    template<auto Candidate, std::size_t... Index>
    [[nodiscard]]
    auto wrap(std::index_sequence<Index...>) noexcept {
        return [](const void*, Args... args) -> Ret {
            [[maybe_unused]]
            const auto arguments = std::forward_as_tuple(std::forward<Args>(args)...);
            return static_cast<Ret>(std::invoke(Candidate, std::forward<type_list_element_t<Index, type_list<Args...>>>(std::get<Index>(arguments))...));
        };
    }

    template<auto Candidate, typename Type, std::size_t... Index>
    [[nodiscard]]
    auto wrap(Type&, std::index_sequence<Index...>) noexcept {
        return [](const void* payload, Args... args) -> Ret {
            [[maybe_unused]]
            const auto arguments = std::forward_as_tuple(std::forward<Args>(args)...);
            Type* curr = static_cast<Type*>(const_cast<constness_as_t<void, Type> *>(payload));
            return static_cast<Ret>(std::invoke(Candidate, *curr, std::forward<type_list_element_t<Index, type_list<Args...>>>(std::get<Index>(arguments))...));
        };
    }

    template<auto Candidate, typename Type, std::size_t... Index>
    [[nodiscard]]
    auto wrap(Type*, std::index_sequence<Index...>) noexcept {
        return [](const void* payload, Args... args) -> Ret {
            [[maybe_unused]]
            const auto arguments = std::forward_as_tuple(std::forward<Args>(args)...);
            Type* curr = static_cast<Type*>(const_cast<constness_as_t<void, Type> *>(payload));
            return static_cast<Ret>(std::invoke(Candidate, curr, std::forward<type_list_element_t<Index, type_list<Args...>>>(std::get<Index>(arguments))...));
        };
    }

public:
    using function_type = Ret(const void*, Args...);
    using type = Ret(Args...);
    using result_type = Ret;

    delegate() noexcept : fn{ nullptr }, data{ nullptr } {}

	template<auto Function>
	delegate(connect_arg_t<Function>) : delegate{} {
		connect<Function>();
	}

    template<auto Candidate, typename Type>
    delegate(connect_arg_t<Candidate>, Type&& value_or_instance) noexcept {
        connect<Candidate>(std::forward<Type>(value_or_instance));
    }

    delegate(function_type* function, const void* payload = nullptr) noexcept {
        connect(function, payload);
    }

    template<auto Candidate>
    void connect() noexcept {
        data = nullptr;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Candidate), Args...>) {
            fn = [](const void*, Args... args) -> Ret {
                return Ret(std::invoke(Candidate, std::forward<Args>(args)...));
            };
        }
        else if constexpr (std::is_member_pointer_v<decltype(Candidate)>) {
            fn = wrap<Candidate>(internal::index_sequence_for<type_list_element_t<0, type_list<Args...>>>(internal::function_pointer_t<decltype(Candidate)>{}));
        }
        else {
            fn = wrap<Candidate>(internal::index_sequence_for(internal::function_pointer_t<decltype(Candidate)>{}));
        }
    }

    template<auto Candidate, typename Type>
    void connect(Type& value_or_instance) noexcept {
        data = &value_or_instance;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Candidate), Type&, Args...>) {
            fn = [](const void* payload, Args... args) -> Ret {
                Type* curr = static_cast<Type*>(const_cast<constness_as_t<void, Type> *>(payload));
                return Ret(std::invoke(Candidate, *curr, std::forward<Args>(args)...));
            };
        }
        else {
            fn = wrap<Candidate>(value_or_instance, internal::index_sequence_for(internal::function_pointer_t<decltype(Candidate), Type>{}));
        }
    }

    template<auto Candidate, typename Type>
    void connect(Type* value_or_instance) noexcept {
        data = value_or_instance;
        if constexpr (std::is_invocable_r_v<Ret, decltype(Candidate), Type*, Args...>) {
            fn = [](const void* payload, Args... args) -> Ret {
                Type* curr = static_cast<Type*>(const_cast<constness_as_t<void, Type> *>(payload));
                return Ret(std::invoke(Candidate, curr, std::forward<Args>(args)...));
            };
        }
        else {
            fn = wrap<Candidate>(value_or_instance, internal::index_sequence_for(internal::function_pointer_t<decltype(Candidate), Type>{}));
        }
    }

    void connect(function_type* function, const void* payload = nullptr) noexcept {
        fn = function;
        data = payload;
    }

    void reset() noexcept {
        fn = nullptr;
        data = nullptr;
    }

    [[nodiscard]]
    const void* instance() const noexcept {
        return data;
    }

    Ret operator()(Args... args) const {
        return fn(data, std::forward<Args>(args)...);
    }

    [[nodiscard]]
    explicit operator bool() const noexcept {
        return !(fn == nullptr);
    }

    [[nodiscard]]
    bool operator==(const delegate<Ret(Args...)>& other) const noexcept {
        return fn == other.fn && data == other.data;
    }

private:
    function_type* fn;
    const void* data;
};

template<typename Ret, typename... Args>
[[nodiscard]]
bool operator!=(const delegate<Ret(Args...)>& lhs, const delegate<Ret(Args...)>& rhs) noexcept {
    return !(lhs == rhs);
}

template<auto Candidate>
delegate(connect_arg_t<Candidate>) noexcept -> delegate<std::remove_pointer_t<internal::function_pointer_t<decltype(Candidate)>>>;

template<auto Candidate, typename Type>
delegate(connect_arg_t<Candidate>, Type&&) noexcept -> delegate<std::remove_pointer_t<internal::function_pointer_t<decltype(Candidate), Type>>>;

template<typename Ret, typename... Args>
delegate(Ret(*)(const void*, Args...), const void* = nullptr) noexcept -> delegate<Ret(Args...)>;

template <class BaseClass, class IdType>
class GenericObjectFactory {
	std::unordered_map<IdType, BaseClass* (*)()> m_creationFunctions;
	std::unordered_map<IdType, std::string> m_names;

public:
	template <class SubClass>
	bool Register(IdType id) {
		auto findIt = m_creationFunctions.find(id);
		if (findIt == m_creationFunctions.end()) {
			m_creationFunctions[id] = []()->BaseClass* { return new SubClass; };
			return true;
		}

		return false;
	}

	template <class SubClass>
	bool Register(IdType id, const std::string& name) {
		auto findIt = m_creationFunctions.find(id);
		if (findIt == m_creationFunctions.end()) {
			m_creationFunctions[id] = []()->BaseClass* { return new SubClass; };
			m_names[id] = name;
			return true;
		}

		return false;
	}

	BaseClass* Create(IdType id) {
		auto findIt = m_creationFunctions.find(id);
		if (findIt != m_creationFunctions.end()) {
			auto pFunc = findIt->second;
			return pFunc();
		}

		return nullptr;
	}

	const std::string& GetName(IdType id) {
		auto findIt = m_names.find(id);
		if (findIt != m_names.end()) {
			return findIt->second;
		}
		return "No name or not registered";
	}
};

class IEventData;

using EventTypeId = unsigned long;
using IEventDataPtr = std::shared_ptr<IEventData>;
using EventListenerDelegate = delegate<void(IEventDataPtr)>;

class IEventData {
public:
	virtual ~IEventData() = default;
	virtual const EventTypeId& VGetEventType() const = 0;
	virtual const std::string& GetName() const = 0;
};

class EvtData_Before_Start_Processing : public IEventData {
	std::ostream& m_out;
public:
	static const EventTypeId sk_EventType = 0x8E2AD6E6;
	static const std::string sk_EventName;

	EvtData_Before_Start_Processing();
	EvtData_Before_Start_Processing(std::ostream& out);

	const EventTypeId& VGetEventType() const override;
	const std::string& GetName() const override;

	std::ostream& GetOutput();

	friend std::ostream& operator<<(std::ostream& os, const EvtData_Before_Start_Processing& evt);
};

std::ostream& operator<<(std::ostream& os, const EvtData_Before_Start_Processing& evt);

class EvtData_After_End_Processing : public IEventData {
	std::ostream& m_out;
public:
	static const EventTypeId sk_EventType = 0x77DD2B3A;
	static const std::string sk_EventName;

	EvtData_After_End_Processing();
	EvtData_After_End_Processing(std::ostream& out);

	const EventTypeId& VGetEventType() const override;
	const std::string& GetName() const override;

	std::ostream& GetOutput();

	friend std::ostream& operator<<(std::ostream& os, const EvtData_After_End_Processing& evt);
};

std::ostream& operator<<(std::ostream& os, const EvtData_After_End_Processing& evt);

class EvtData_Before_User_Data_Processing : public IEventData {
	bool m_last_entry;
	bool m_first_entry;
	std::ostream& m_out;
public:
	static const EventTypeId sk_EventType = 0xE86C7C31;
	static const std::string sk_EventName;

	EvtData_Before_User_Data_Processing();
	EvtData_Before_User_Data_Processing(std::ostream& out, bool last_entry, bool first_entry);

	const EventTypeId& VGetEventType() const override;
	const std::string& GetName() const override;

	std::ostream& GetOutput();
	bool IslastEntry();
	bool IsFirstEntry();

	friend std::ostream& operator<<(std::ostream& os, const EvtData_Before_User_Data_Processing& evt);
};

std::ostream& operator<<(std::ostream& os, const EvtData_Before_User_Data_Processing& evt);

class EvtData_After_User_Data_Processing : public IEventData {
	bool m_last_entry;
	bool m_first_entry;
	std::ostream& m_out;
public:
	static const EventTypeId sk_EventType = 0xEEAA0A40;
	static const std::string sk_EventName;

	EvtData_After_User_Data_Processing();
	EvtData_After_User_Data_Processing(std::ostream& out, bool last_entry, bool first_entry);

	const EventTypeId& VGetEventType() const override;
	const std::string& GetName() const override;

	std::ostream& GetOutput();
	bool IslastEntry();
	bool IsFirstEntry();

	friend std::ostream& operator<<(std::ostream& os, const EvtData_After_User_Data_Processing& evt);
};

std::ostream& operator<<(std::ostream& os, const EvtData_After_User_Data_Processing& evt);

extern GenericObjectFactory<IEventData, EventTypeId> g_eventFactory;
#define REGISTER_EVENT(eventClass) g_eventFactory.Register<eventClass>(eventClass::sk_EventType, eventClass::sk_EventName)
#define CREATE_EVENT(eventType) g_eventFactory.Create(eventType)
#define GET_EVENT_NAME(eventType) g_eventFactory.GetName(eventType)

class IEventManager {
public:

	explicit IEventManager(bool setAsGlobal);
	virtual ~IEventManager();

	virtual bool VAddListener(const EventListenerDelegate& eventDelegate, const EventTypeId& type) = 0;
	virtual bool VRemoveListener(const EventListenerDelegate& eventDelegate, const EventTypeId& type) = 0;
	virtual bool VTriggerEvent(const IEventDataPtr& pEvent) const = 0;
	virtual bool VQueueEvent(const IEventDataPtr& pEvent) = 0;
	virtual bool VUpdate() = 0;
	virtual bool VAbortEvent(const EventTypeId& type, bool allOfType = false) = 0;

	static IEventManager* Get();
	static IEventDataPtr Create(EventTypeId eventType);
};

const unsigned int EVENTMANAGER_NUM_QUEUES = 2;

class EventManager : public IEventManager {
	std::unordered_map<EventTypeId, std::list<EventListenerDelegate>> m_eventListeners;
	const std::string m_eventManagerName;

	std::list<IEventDataPtr> m_queues[EVENTMANAGER_NUM_QUEUES];
	int m_activeQueue;

public:
	explicit EventManager(const std::string& pName, bool setAsGlobal);

	bool VAddListener(const EventListenerDelegate& eventDelegate, const EventTypeId& type) override;
	bool VRemoveListener(const EventListenerDelegate& eventDelegate, const EventTypeId& type) override;

	bool VTriggerEvent(const IEventDataPtr& pEvent) const override;
	bool VQueueEvent(const IEventDataPtr& pEvent) override;
	bool VUpdate() override;
	bool VAbortEvent(const EventTypeId& inType, bool allOfType) override;

	friend std::ostream& operator<<(std::ostream& os, const EventManager& mgr);
};

std::ostream& operator<<(std::ostream& os, const EventManager& mgr);