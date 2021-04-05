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
    
	TransportCatalogue tc;
	const json::Document doc = json::Load(std::cin);
	
	std::unique_ptr<IOReaderJson> ioReaderJson = IOReaderFactory::Create<IOReaderJson>();
	std::vector<std::unique_ptr<UserInputData>> inputData = ioReaderJson->getUserInput(doc);
	InputDataProcessor::Process(tc, std::move(inputData));
	
	std::vector<std::unique_ptr<UserStatData>> statData = ioReaderJson->getUserStat(doc);
	StatDataProcessor proc = StatDataProcessorFactory::Create(StreamType::JSON);
	proc.Process(tc, std::move(statData), std::cout);
	

	return 0;
}