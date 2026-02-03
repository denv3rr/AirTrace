#ifndef CORE_PROVENANCE_H
#define CORE_PROVENANCE_H

#include <string>

enum class ProvenanceTag
{
    Operational,
    Simulation,
    Test,
    Unknown
};

enum class UnknownProvenanceAction
{
    Deny,
    Hold
};

inline const char *provenanceToString(ProvenanceTag tag)
{
    switch (tag)
    {
    case ProvenanceTag::Operational:
        return "operational";
    case ProvenanceTag::Simulation:
        return "simulation";
    case ProvenanceTag::Test:
        return "test";
    case ProvenanceTag::Unknown:
    default:
        return "unknown";
    }
}

#endif // CORE_PROVENANCE_H
