#pragma once

#include "Detector.h"

void GetDeviceList(kmk::ValidDeviceIdentifierVector &listOut)
{
    listOut.reserve(8);

    // GR1/A
    kmk::ValidDeviceIdentifier gr1Dev = {0x0, 0x4d8};
    kmk::ValidDeviceIdentifier gr1ADev = {0x101, 0x4d8};
    listOut.push_back(gr1Dev);
    listOut.push_back(gr1ADev);

    // K102
    kmk::ValidDeviceIdentifier k102Dev = {0x011, 0x4d8};
    listOut.push_back(k102Dev);

    // Radangel
    kmk::ValidDeviceIdentifier angelDev = {0x100, 0x4d8};
    listOut.push_back(angelDev);

    // Sigma25
    kmk::ValidDeviceIdentifier sig25Dev = {0x022, 0x4d8};
    listOut.push_back(sig25Dev);

    // Sigma50
    kmk::ValidDeviceIdentifier sig50Dev = {0x023, 0x4d8};
    listOut.push_back(sig50Dev);

    // TN15
    kmk::ValidDeviceIdentifier tn15Dev = {0x030, 0x4d8};
    listOut.push_back(tn15Dev);

    // GR05
    kmk::ValidDeviceIdentifier gr05Dev = {0x050, 0x2A5A};
    listOut.push_back(gr05Dev);
}
