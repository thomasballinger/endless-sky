// quick_example.cpp
#include <emscripten/bind.h>

#include "source/Random.h"
#include "source/DataNode.h"
#include "source/Angle.h"
#include "source/FormationPattern.h"
#include "source/Point.h"

#include "tests/include/datanode-factory.h"

#include <iostream>
using namespace emscripten;

void randSeed(int seed) {
    const uint64_t s = (uint64_t) seed;
    Random::Seed(s);
}

int randInt(int mod) {
  const uint32_t m = (uint64_t) mod;
  auto raw = Random::Int(m);
  const unsigned int smallValue = (unsigned int) raw;
  return smallValue;
}


// DataNode factory
EMSCRIPTEN_BINDINGS(somethingwhatgoeshere) {
    function("AsDataNode", &AsDataNode);
}

// DataNode
EMSCRIPTEN_BINDINGS(my_class_example) {
  class_<DataNode>("DataNode")
    .constructor<const DataNode*>()
    .function("Size", &DataNode::Size)
    .function("Value", (double(DataNode::*)(int) const)&DataNode::Value)
    .function("IsNumber", (bool(DataNode::*)(int) const)&DataNode::IsNumber)
    .function("HasChildren", &DataNode::Size)
    .function("PrintTrace", &DataNode::PrintTrace)
    ;
}

// Angle
EMSCRIPTEN_BINDINGS(my_class_example2) {
  class_<Angle>("Angle")
    .constructor<>()
    .function("Degrees", &Angle::Degrees)
    ;
}

// FormationPattern
EMSCRIPTEN_BINDINGS(my_class_example3) {
  class_<FormationPattern>("FormationPattern")
    .constructor<>()
    .function("Load", &FormationPattern::Load)
    .function("Name", &FormationPattern::Name)
    .function("SetName", &FormationPattern::SetName)
    .function("begin", &FormationPattern::begin)
    .function("Lines", &FormationPattern::Lines)
    .function("Repeats", &FormationPattern::Repeats)
    .function("Slots", &FormationPattern::Slots)
    .function("IsCentered", &FormationPattern::IsCentered)
    .function("Position", &FormationPattern::Position)
    .function("Rotatable", &FormationPattern::Rotatable)
    .function("FlippableY", &FormationPattern::FlippableY)
    .function("FlippableX", &FormationPattern::FlippableX)
    ;
}

std::vector<Point> GetNPositions(std::string text, int startRing, int numShips) {
    auto node = AsDataNode(text);
    FormationPattern formationPat;
    formationPat.Load(node);
    std::cout << formationPat.Name();

    FormationPattern::ActiveFormation af;
    auto it = formationPat.begin(af, startRing, numShips);

    int i = 0;
    std::vector<Point> positions(numShips);
    while (i < numShips) {
      auto point = it;
      positions[i] = *point;
      std::cout << "(" << (*point).Xval() << ", " << (*point).Yval() << ")";
      ++i;
      ++it;
    }

    return positions;
}

// Point
EMSCRIPTEN_BINDINGS(my_class_example4) {
  class_<Point>("Point")
    .constructor<double, double>()
    .function("X", &Point::Xval)
    .function("Y", &Point::Yval)
    .function("Set", &Point::Set)
    ;
  register_vector<Point>("PointVec");
  function("GetNPositions", &GetNPositions);
  function("randSeed", &randSeed);
  function("randInt", &randInt);
}
