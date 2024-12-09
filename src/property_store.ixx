// SPDX-FileCopyrightText: © 2024 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

export module property_store;

import <win.hpp>;

export HRESULT create_property_store_class_factory(GUID const& interface_id, void** result);
