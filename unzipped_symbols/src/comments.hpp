
/**
 * @file comments.hpp
 * @author Hartley blakey
 * @brief Library to remove the comments from a ChagaLite source file
 */

#ifndef COMMENTS_HPP
#define COMMENTS_HPP

#include <string>

/**
 * @brief Function to remove the comments from ChagaLite source code
 * 
 * @param file The contents of the file
 * @return Any errors, or the empty String if the operation completed successfully
 * 
 * @post If the function returned an empty std::string, file is now ChagaLite source
 * code with all commented out characters replaced by spaces
 */
std::string removeComments(std::string& file);


#endif /* COMMENTS_HPP */
