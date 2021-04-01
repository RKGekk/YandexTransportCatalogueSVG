#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <functional>
#include <memory>
#include <vector>

#include "transport_catalogue.h"
#include "domain.h"
#include "map_renderer.h"


/*
 * ����� ����� ���� �� ���������� ��� ����������� �������� � ����, ����������� ������, ������� ��
 * �������� �� �������� �� � transport_catalogue, �� � json reader.
 *
 * � �������� ��������� ��� ���� ���������� ��������� �� ���� ������ ����������� ��������.
 * �� ������ ����������� ��������� �������� ��������, ������� ������� ���.
 *
 * ���� �� ������������� �������, ��� ����� ���� �� ��������� � ���� ����,
 * ������ �������� ��� ������.
 */

 // ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
 // � ������� ������������ ����������.
 // ��. ������� �������������� �����: https://ru.wikipedia.org/wiki/�����_(������_��������������)
 /*
 class RequestHandler {
 public:
     // MapRenderer ����������� � ��������� ����� ��������� �������
     RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

     // ���������� ���������� � �������� (������ Bus)
     std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

     // ���������� ��������, ���������� �����
     const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

     // ���� ����� ����� ����� � ��������� ����� ��������� �������
     svg::Document RenderMap() const;

 private:
     // RequestHandler ���������� ��������� �������� "������������ ����������" � "������������ �����"
     const TransportCatalogue& db_;
     const renderer::MapRenderer& renderer_;
 };
 */

class InputDataProcessor {
public:
    static void Process(TransportCatalogue& transport_catalog, std::vector<std::unique_ptr<UserInputData>>);
};

class StatDataProcessor {
public:
    using ProcessFn = std::function<void(const TransportCatalogue&, const std::unique_ptr<UserStatData>&, std::ostream&)>;

    StatDataProcessor();

    void Process(const TransportCatalogue& transport_catalog, std::vector<std::unique_ptr<UserStatData>>, std::ostream& out);
    int RegisterProcess(StatRequestType rt, ProcessFn fn);
    void RegisterEventListener(const EventListenerDelegate& eventDelegate, const EventTypeId& type);
private:
    std::unordered_map<StatRequestType, std::unordered_map<int, ProcessFn>> _processes;
    std::unique_ptr<IEventManager> m_evt_mgr;
    static int _ct;
};

class StatDataProcessorFactory {
public:
    static StatDataProcessor Create(StreamType st);
};