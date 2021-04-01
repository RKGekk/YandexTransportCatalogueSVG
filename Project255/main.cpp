#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <memory>

#include "domain.h"
#include "transport_catalogue.h"
#include "geo.h"
#include "json.h"
#include "json_reader.h"
#include "request_handler.h"

int main() {
	using namespace std::literals;
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массива "stat_requests", построив JSON-массив
     * с ответами Вывести в stdout ответы в виде JSON
     */


	{
		TransportCatalogue tc;
		tc.addRouteStop("Marushkino", Coordinates{ 55.595884, 37.209755 }, {});
		tc.addRoute("256"s, Route{ {"Biryulyovo Zapadnoye", "Biryusinka", "Universam", "Biryulyovo Tovarnaya", "Biryulyovo Passazhirskaya"}, true });
		tc.addRoute("750"s, Route{ {"Tolstopaltsevo", "Marushkino", "Rasskazovka"}, false });
		tc.addRouteStop("Rasskazovka", Coordinates{ 55.632761, 37.333324 }, {});
		tc.addRouteStop("Biryulyovo Zapadnoye", Coordinates{ 55.574371, 37.651700 }, {});
		tc.addRouteStop("Biryusinka", Coordinates{ 55.581065, 37.648390 }, {});
		tc.addRouteStop("Universam", Coordinates{ 55.587655, 37.645687 }, {});
		tc.addRouteStop("Biryulyovo Tovarnaya", Coordinates{ 55.592028, 37.653656 }, {});
		tc.addRouteStop("Biryulyovo Passazhirskaya", Coordinates{ 55.580999, 37.659164 }, {});
	}

	{
		//TransportCatalogue tc;
		//std::stringstream myString;
		////myString << "10\nStop Tolstopaltsevo: 55.611087, 37.208290\nStop Marushkino: 55.595884, 37.209755\nBus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\nBus 750: Tolstopaltsevo - Marushkino - Rasskazovka\nStop Rasskazovka: 55.632761, 37.333324\nStop Biryulyovo Zapadnoye: 55.574371, 37.651700\nStop Biryusinka: 55.581065, 37.648390\nStop Universam: 55.587655, 37.645687\nStop Biryulyovo Tovarnaya: 55.592028, 37.653656\nStop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n3\nBus 256\nBus 750\nBus 751";
		////myString << "13\nStop Tolstopaltsevo: 55.611087, 37.20829\nStop Marushkino: 55.595884, 37.209755\nBus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\nBus 750: Tolstopaltsevo - Marushkino - Rasskazovka\nStop Rasskazovka: 55.632761, 37.333324\nStop Biryulyovo Zapadnoye: 55.574371, 37.6517\nStop Biryusinka: 55.581065, 37.64839\nStop Universam: 55.587655, 37.645687\nStop Biryulyovo Tovarnaya: 55.592028, 37.653656\nStop Biryulyovo Passazhirskaya: 55.580999, 37.659164\nBus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\nStop Rossoshanskaya ulitsa: 55.595579, 37.605757\nStop Prazhskaya: 55.611678, 37.603831\n6\nBus 256\nBus 750\nBus 751\nStop Samara\nStop Prazhskaya\nStop Biryulyovo Zapadnoye\n";
		////myString << "13\nStop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\nStop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\nBus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\nBus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\nStop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\nStop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\nStop Biryusinka: 55.581065, 37.64839, 750m to Universam\nStop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\nStop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\nStop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\nBus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\nStop Rossoshanskaya ulitsa: 55.595579, 37.605757\nStop Prazhskaya: 55.611678, 37.603831\n6\nBus 256\nBus 750\nBus 751\nStop Samara\nStop Prazhskaya\nStop Biryulyovo Zapadnoye\n";
		//myString << "13\nStop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino\nStop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino\nBus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\nBus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka\nStop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino\nStop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam\nStop Biryusinka: 55.581065, 37.64839, 750m to Universam\nStop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya\nStop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya\nStop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye\nBus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye\nStop Rossoshanskaya ulitsa: 55.595579, 37.605757\nStop Prazhskaya: 55.611678, 37.603831\n6\nBus 256\nBus 750\nBus 751\nStop Samara\nStop Prazhskaya\nStop Biryulyovo Zapadnoye\n";

		//std::unique_ptr<InputReaderText> inputReaderText = InputReaderFactory::Create<InputReaderText>();
		//std::vector<std::unique_ptr<UserInputData>> inputData = inputReaderText->getUserInput(myString);
		//InputDataProcessor::Process(tc, std::move(inputData));

		//std::unique_ptr<StatReaderText> statReader = StatReaderFactory::Create<StatReaderText>();
		//std::vector<std::unique_ptr<UserStatData>> statData = statReader->getUserStat(myString);

		//StatDataProcessor proc = StatDataProcessorFactory::Create(StreamType::TEXT);
		//proc.Process(tc, std::move(statData), std::cout);
	}

	/*{
		TransportCatalogue tc;
		std::stringstream myString;
		myString << R"(
			{
			  "base_requests": [
			    {
			      "type": "Bus",
			      "name": "114",
			      "stops": ["Морской вокзал", "Ривьерский мост"],
			      "is_roundtrip": false
			    },
			    {
			      "type": "Stop",
			      "name": "Ривьерский мост",
			      "latitude": 43.587795,
			      "longitude": 39.716901,
			      "road_distances": {"Морской вокзал": 850}
			    },
			    {
			      "type": "Stop",
			      "name": "Морской вокзал",
			      "latitude": 43.581969,
			      "longitude": 39.719848,
			      "road_distances": {"Ривьерский мост": 850}
			    }
			  ],
			  "stat_requests": [
			    { "id": 1, "type": "Stop", "name": "Ривьерский мост" },
			    { "id": 2, "type": "Bus", "name": "114" }
			  ]
			}
		)";

		const json::Document doc = json::Load(myString);

		std::unique_ptr<IOReaderJson> ioReaderJson = IOReaderFactory::Create<IOReaderJson>();
		std::vector<std::unique_ptr<UserInputData>> inputData = ioReaderJson->getUserInput(doc);
		InputDataProcessor::Process(tc, std::move(inputData));

		std::vector<std::unique_ptr<UserStatData>> statData = ioReaderJson->getUserStat(doc);
		StatDataProcessor proc = StatDataProcessorFactory::Create(StreamType::JSON);
		proc.Process(tc, std::move(statData), std::cout);
	}*/

	/*{
		TransportCatalogue tc;
		std::stringstream myString;
		myString << R"(
			{
			  "base_requests": [
			    {
			      "type": "Bus",
			      "name": "114",
			      "stops": ["Морской вокзал", "Ривьерский мост"],
			      "is_roundtrip": false
			    },
			    {
			      "type": "Stop",
			      "name": "Ривьерский мост",
			      "latitude": 43.587795,
			      "longitude": 39.716901,
			      "road_distances": {"Морской вокзал": 850}
			    },
			    {
			      "type": "Stop",
			      "name": "Морской вокзал",
			      "latitude": 43.581969,
			      "longitude": 39.719848,
			      "road_distances": {"Ривьерский мост": 850}
			    }
			  ],
			  "render_settings": {
			    "width": 200,
			    "height": 200,
			    "padding": 30,
			    "stop_radius": 5,
			    "line_width": 14,
			    "bus_label_font_size": 20,
			    "bus_label_offset": [7, 15],
			    "stop_label_font_size": 20,
			    "stop_label_offset": [7, -3],
			    "underlayer_color": [255,255,255,0.85],
			    "underlayer_width": 3,
			    "color_palette": ["green", [255,160,0],"red"]
			  },
			  "stat_requests": [
			    { "id": 1, "type": "Map" },
			    { "id": 2, "type": "Stop", "name": "Ривьерский мост" },
			    { "id": 3, "type": "Bus", "name": "114" }
			  ]
			}
		)";

		const json::Document doc = json::Load(myString);

		std::unique_ptr<IOReaderJson> ioReaderJson = IOReaderFactory::Create<IOReaderJson>();
		std::vector<std::unique_ptr<UserInputData>> inputData = ioReaderJson->getUserInput(doc);
		InputDataProcessor::Process(tc, std::move(inputData));

		std::vector<std::unique_ptr<UserStatData>> statData = ioReaderJson->getUserStat(doc);
		StatDataProcessor proc = StatDataProcessorFactory::Create(StreamType::JSON);
		proc.Process(tc, std::move(statData), std::cout);
	}*/

	{
		TransportCatalogue tc;
		std::stringstream myString;
		myString << R"(
			{
			  "base_requests": [
			    {
			      "type": "Bus",
			      "name": "14",
			      "stops": [
			        "Улица Лизы Чайкиной",
			        "Электросети",
			        "Улица Докучаева",
			        "Улица Лизы Чайкиной"
			      ],
			      "is_roundtrip": true
			    },
			    {
			      "type": "Bus",
			      "name": "114",
			      "stops": [
			        "Морской вокзал",
			        "Ривьерский мост"
			      ],
			      "is_roundtrip": false
			    },
			    {
			      "type": "Stop",
			      "name": "Ривьерский мост",
			      "latitude": 43.587795,
			      "longitude": 39.716901,
			      "road_distances": {
			        "Морской вокзал": 850
			      }
			    },
			    {
			      "type": "Stop",
			      "name": "Морской вокзал",
			      "latitude": 43.581969,
			      "longitude": 39.719848,
			      "road_distances": {
			        "Ривьерский мост": 850
			      }
			    },
			    {
			      "type": "Stop",
			      "name": "Электросети",
			      "latitude": 43.598701,
			      "longitude": 39.730623,
			      "road_distances": {
			        "Улица Докучаева": 3000,
			        "Улица Лизы Чайкиной": 4300
			      }
			    },
			    {
			      "type": "Stop",
			      "name": "Улица Докучаева",
			      "latitude": 43.585586,
			      "longitude": 39.733879,
			      "road_distances": {
			        "Улица Лизы Чайкиной": 2000,
			        "Электросети": 3000
			      }
			    },
			    {
			      "type": "Stop",
			      "name": "Улица Лизы Чайкиной",
			      "latitude": 43.590317,
			      "longitude": 39.746833,
			      "road_distances": {
			        "Электросети": 4300,
			        "Улица Докучаева": 2000
			      }
			    }
			  ],
			  "render_settings": {
			    "width": 600,
			    "height": 400,
			    "padding": 50,
			    "stop_radius": 5,
			    "line_width": 14,
			    "bus_label_font_size": 20,
			    "bus_label_offset": [
			      7,
			      15
			    ],
			    "stop_label_font_size": 20,
			    "stop_label_offset": [
			      7,
			      -3
			    ],
			    "underlayer_color": [
			      255,
			      255,
			      255,
			      0.85
			    ],
			    "underlayer_width": 3,
			    "color_palette": [
			      "green",
			      [
			        255,
			        160,
			        0
			      ],
			      "red"
			    ]
			  },
			  "stat_requests": [
			    {
			      "id": 1218663236,
			      "type": "Map"
			    }
			  ]
			} 
		)";

		const json::Document doc = json::Load(myString);
		
		std::unique_ptr<IOReaderJson> ioReaderJson = IOReaderFactory::Create<IOReaderJson>();
		std::vector<std::unique_ptr<UserInputData>> inputData = ioReaderJson->getUserInput(doc);
		InputDataProcessor::Process(tc, std::move(inputData));
		
		std::vector<std::unique_ptr<UserStatData>> statData = ioReaderJson->getUserStat(doc);
		StatDataProcessor proc = StatDataProcessorFactory::Create(StreamType::JSON);
		proc.Process(tc, std::move(statData), std::cout);
	}

	return 0;
}