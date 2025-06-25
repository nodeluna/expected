/*
 * Copyright: 2025 nodeluna
 * SPDX-License-Identifier: Apache-2.0
 * repository: https://github.com/nodeluna/expected
 */

#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <new>
#include <stdexcept>

namespace nl {

	struct monostate {};

	template<class T = monostate, class E = monostate>
	class expected {
		private:
			bool _has_value = false;

			union {
					T _value;
					E _error;
			};

		public:
			expected(const T& t) : _has_value(true)
			{
				new (std::addressof(_value)) T(t);
			}

			expected(const E& e) : _has_value(false)
			{
				new (std::addressof(_error)) E(e);
			}

			expected() : _has_value(true)
			{
				new (std::addressof(_value)) T();
			}

			expected& operator=(const expected& other)
			{
				if (this != &other)
				{
					this->~expected();
					new (this) expected(other);
				}

				return *this;
			}

			template<class U>
			expected(const expected<U, E>& other) : _has_value(other.has_value())
			{
				if (_has_value)
				{
					if constexpr (std::is_same<U, monostate>::value)
					{
						new (std::addressof(_value)) T();
					}
					else if constexpr (std::is_same<U, T>::value)
					{
						new (std::addressof(_value)) T(other.value());
					}
					else
					{
						static_assert(
						    not std::is_same<U, T>::value, "no available conversion between the provided value types");
					}
				}
				else
				{
					new (std::addressof(_error)) E(other.error());
				}
			}

			expected(const expected& other) : _has_value(other.has_value())
			{
				if (_has_value)
				{
					new (std::addressof(_value)) T(other.value());
				}
				else
				{
					new (std::addressof(_error)) E(other.error());
				}
			}

			expected(const expected&& other) noexcept : _has_value(other._has_value)
			{
				if (this->has_value())
					new (std::addressof(_value)) T(std::move(other._value));
				else
					new (std::addressof(_error)) E(std::move(other._error));
			}

			expected& operator=(const expected&& other) noexcept
			{
				if (this != &other)
				{
					this->~expected();
					new (this) expected(std::move(other));
				}

				return *this;
			}

			~expected()
			{
				if (this->has_value())
					_value.~T();
				else
					_error.~E();
			}

			bool has_value() const noexcept
			{
				return _has_value;
			}

			explicit operator bool() const noexcept
			{
				return this->has_value();
			}

			constexpr const T& value() const&
			{
				if (not _has_value)
				{
					throw std::runtime_error("Attempted to access the value of a error state");
				}
				return _value;
			}

			constexpr const E& error() const&
			{
				if (_has_value)
				{
					throw std::runtime_error("Attempted to access the error of a value state");
				}
				return _error;
			}

			constexpr T& value() &
			{
				if (not _has_value)
				{
					throw std::runtime_error("Attempted to access the value of a error state");
				}
				return _value;
			}

			constexpr E& error() &
			{
				if (_has_value)
				{
					throw std::runtime_error("Attempted to access the error of a value state");
				}
				return _error;
			}

			constexpr const T&& value() const&&
			{
				if (not _has_value)
				{
					throw std::runtime_error("Attempted to access the value of a error state");
				}
				return std::move(_value);
			}

			constexpr const E&& error() const&&
			{
				if (_has_value)
				{
					throw std::runtime_error("Attempted to access the error of a value state");
				}
				return std::move(_error);
			}

			constexpr T&& value() &&
			{
				if (not _has_value)
				{
					throw std::runtime_error("Attempted to access the value of a error state");
				}
				return std::move(_value);
			}

			constexpr E&& error() &&
			{
				if (_has_value)
				{
					throw std::runtime_error("Attempted to access the error of a value state");
				}
				return std::move(_error);
			}

			template<class U = std::remove_cv_t<T>>
			constexpr T value_or(U&& other) const&
			{
				static_assert(std::is_convertible<U, T>::value, "the provided type must be convertible to the value type");
				if (_has_value)
					return _value;
				else
					return static_cast<T>(std::forward<U>(other));
			}

			template<class U = std::remove_cv_t<E>>
			constexpr E error_or(U&& other) const&
			{
				static_assert(std::is_convertible<U, E>::value, "the provided type must be convertible to the error type");
				if (not _has_value)
					return _error;
				else
					return static_cast<E>(std::forward<U>(other));
			}
	};

	template<class E>
	expected<monostate, E> unexpected(const E& e)
	{
		return expected<monostate, E>(e);
	}

	expected<monostate, std::string> unexpected(const char* e)
	{
		return unexpected<std::string>(std::string(e));
	}
}
