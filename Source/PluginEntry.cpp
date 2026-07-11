#include "FinisherProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FinisherProcessor();
}
