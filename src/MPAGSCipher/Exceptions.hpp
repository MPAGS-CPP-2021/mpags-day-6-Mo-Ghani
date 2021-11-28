#ifndef MPAGSCIPHER_EXCEPTIONS_HPP
#define MPAGSCIPHER_EXCEPTIONS_HPP

#include <exception>
#include <string>
#include <iostream>

/**
 * \file Exceptions.hpp
 * \brief Contains the declaration of the exceptions that may be thrown
 */

/**
 * \class MissingArgument
 * \brief Defines the exception thrown when an expected argument is not given
 */
class MissingArgument : public std::out_of_range {
    public:
        /**
         * \brief Create a new NissingArgument with a given error message
         *
         * \param msg the message to be thrown in the case of an exception
         */
        MissingArgument( const std::string& msg ) : std::out_of_range(msg) {}
};

/**
 * \class UnknownArgument
 * \brief Defines the exception thrown when an unexpected argument is given
 */
class UnknownArgument : public std::invalid_argument {
    public:
        /**
         * \brief Create a new UnknownArgument with a given error message
         *
         * \param msg the message to be thrown in the case of an exception
         */
        UnknownArgument( const std::string& msg ) : std::invalid_argument(msg) {}
};

/**
 * \class InvalidKey
 * \brief Defines the exception thrown when an invalid argument is provided
 */
class InvalidKey : public std::logic_error{
    public:
        /**
         * \brief Create a new InvalidKey with an error argument
         *
         * \param msg the message to be thrown in the case of an 
         *            invalid argument exception
         */
        InvalidKey(const std::string& msg) : std::logic_error(msg) {}
};


#endif