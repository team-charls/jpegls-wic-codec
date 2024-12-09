// SPDX-FileCopyrightText: © 2020 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

#include <CppUnitTest.h>

import <win.hpp>;

import property_variant;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(property_variant_test)
{
public:
    TEST_METHOD(create_default) // NOLINT
    {
        const property_variant variant;

        Assert::AreEqual(static_cast<int>(VT_EMPTY), static_cast<int>(variant.vt));
    }

    TEST_METHOD(create_uint16) // NOLINT
    {
        constexpr uint16_t expected{4000};
        const property_variant variant{expected};

        Assert::AreEqual(static_cast<int>(VT_UI2), static_cast<int>(variant.vt));
        Assert::AreEqual(expected, variant.uiVal);
    }

    TEST_METHOD(create_uint32) // NOLINT
    {
        constexpr uint32_t expected{4000};
        const property_variant variant{expected};

        Assert::AreEqual(static_cast<int>(VT_UI4), static_cast<int>(variant.vt));
        Assert::AreEqual(expected, variant.uintVal);
    }
};
