/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOW_DATAFLOWMETAPATTERN_H_
#define TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOW_DATAFLOWMETAPATTERN_H_

#include "Pattern.h"
#include "traact/traact_core_export.h"
namespace traact::pattern {
/**
 * Contains serveral interconnected patterns.
 * Exposes the final producer and consumer as new ports of itself
 */
class TRAACT_CORE_EXPORT MetaPattern : public Pattern {

 public:
    MetaPattern(const std::string &name, Concurrency concurrency);
    //void addPattern(Pattern::Ptr pattern);
    //const std::set<Pattern::Ptr> &getPatterns();

 protected:
    std::set<Pattern::Ptr> patterns_;

};
}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOW_DATAFLOWMETAPATTERN_H_
