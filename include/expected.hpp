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

#if __cplusplus >= 201703L
#define _constexpr constexpr
#else
#define _constexpr
#endif

#if __cplusplus >= 202002L
#define _constexpr_destructor constexpr
#define _construct_at(location, arg)\
	std::construct_at(location, arg)
#else
#define _constexpr_destructor
#define _construct_at(location, arg)\
	new (location) arg;
#endif

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
			_constexpr expected(const T& t) : _has_value(true)
			{
				_construct_at(std::addressof(_value), T(t));
			}

			_constexpr expected(const E& e) : _has_value(false)
			{
				_construct_at(std::addressof(_error), E(e));
			}

			_constexpr expected() : _has_value(true)
			{
				static_assert(std::is_default_constructible<T>::value, "");
				_construct_at(std::addressof(_value), T());
			}

			_constexpr expected& operator=(const expected& other)
			{
				static_assert(std::is_copy_assignable<T>::value && std::is_copy_assignable<E>::value, "");
				if (this != &other)
				{
					this->~expected();
					_construct_at(this, expected(other));
				}

				return *this;
			}

			template<class U>
			_constexpr expected(const expected<U, E>& other) : _has_value(other.has_value())
			{
				static_assert(std::is_same<U, nl::monostate>::value, "no available conversion between the provided value types");
				static_assert(std::is_copy_constructible<T>::value && std::is_copy_constructible<E>::value, "");
				if (_has_value)
				{
					_construct_at(std::addressof(_value), T());
				}
				else
				{
					_construct_at(std::addressof(_error), E(other.error()));
				}
			}

			_constexpr expected(const expected& other) : _has_value(other.has_value())
			{
				static_assert(std::is_copy_constructible<T>::value && std::is_copy_constructible<E>::value, "");
				if (_has_value)
				{
					_construct_at(std::addressof(_value), T(other.value()));
				}
				else
				{
					_construct_at(std::addressof(_error), E(other.error()));
				}
			}

			_constexpr expected(const expected&& other) noexcept : _has_value(other._has_value)
			{
				static_assert(std::is_move_constructible<T>::value && std::is_move_constructible<E>::value, "");
				if (this->has_value())
				{
					_construct_at(std::addressof(_value), T(std::move(other._value)));
				}
				else
				{
					_construct_at(std::addressof(_error), E(std::move(other._error)));
				}
			}

			_constexpr expected& operator=(const expected&& other) noexcept
			{
				static_assert(std::is_move_assignable<T>::value && std::is_move_assignable<E>::value, "");
				if (this != &other)
				{
					this->~expected();
					_construct_at(this, expected(std::move(other)));
				}

				return *this;
			}
			
			_constexpr_destructor ~expected() 
			{
				if (this->has_value())
					_value.~T();
				else
					_error.~E();
			}

			_constexpr bool has_value() const noexcept
			{
				return _has_value;
			}

			_constexpr explicit operator bool() const noexcept
			{
				return this->has_value();
			}

			_constexpr const T& value() const&
			{
				if (not _has_value)
				{
					throw std::runtime_error("Attempted to access the value of a error state");
				}
				return _value;
			}

			_constexpr const E& error() const&
			{
				if (_has_value)
				{
					throw std::runtime_error("Attempted to access the error of a value state");
				}
				return _error;
			}

			_constexpr T& value() &
			{
				if (not _has_value)
				{
					throw std::runtime_error("Attempted to access the value of a error state");
				}
				return _value;
			}

			_constexpr E& error() &
			{
				if (_has_value)
				{
					throw std::runtime_error("Attempted to access the error of a value state");
				}
				return _error;
			}

			_constexpr const T&& value() const&&
			{
				if (not _has_value)
				{
					throw std::runtime_error("Attempted to access the value of a error state");
				}
				return std::move(_value);
			}

			_constexpr const E&& error() const&&
			{
				if (_has_value)
				{
					throw std::runtime_error("Attempted to access the error of a value state");
				}
				return std::move(_error);
			}

			_constexpr T&& value() &&
			{
				if (not _has_value)
				{
					throw std::runtime_error("Attempted to access the value of a error state");
				}
				return std::move(_value);
			}

			_constexpr E&& error() &&
			{
				if (_has_value)
				{
					throw std::runtime_error("Attempted to access the error of a value state");
				}
				return std::move(_error);
			}

			template<class U = typename std::remove_cv<T>::type>
			_constexpr T value_or(U&& other) const&
			{
				static_assert(std::is_convertible<U, T>::value, "the provided type must be convertible to the value type");
				if (_has_value)
					return _value;
				else
					return static_cast<T>(std::forward<U>(other));
			}

			template<class U = typename std::remove_cv<E>::type>
			_constexpr E error_or(U&& other) const&
			{
				static_assert(std::is_convertible<U, E>::value, "the provided type must be convertible to the error type");
				if (not _has_value)
					return _error;
				else
					return static_cast<E>(std::forward<U>(other));
			}
	};

	template<class E>
	_constexpr_destructor expected<monostate, E> unexpected(const E& e)
	{
		return expected<monostate, E>(e);
	}

	_constexpr_destructor expected<monostate, std::string> unexpected(const char* e)
	{
		return unexpected<std::string>(std::string(e));
	}
}
