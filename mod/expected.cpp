/*
 * Copyright: 2025 nodeluna
 * SPDX-License-Identifier: Apache-2.0
 * repository: https://github.com/nodeluna/ljson
 */

module;

#include <expected.hpp>

export module nl.expected;

export namespace nl {
	using ljson::expected;
	using ljson::unexpected;
	using ljson::monostate;
}
