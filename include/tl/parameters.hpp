#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include <sstream>

namespace tl
{

    struct parameters
    {
        private:
            
            std::stringstream buffer;
            bool has_target;
            bool has_params;

        public:

            parameters() : buffer{}, has_target{false}, has_params{false} 
            {

            }

            parameters(const std::string& target) : buffer{}, has_target{true}, has_params{false}
            {
                buffer << target;
            }

            void clear()
            {
                buffer.str(std::string());
                buffer.clear();
                has_target = has_params = false;
            }

            void target(const std::string& target)
            {
                clear();
                has_target = true;
                buffer << target;
            }

            void set(const std::string& key, const std::string& value)
            {
                if (has_target)
                {
                    if (!has_params)
                        buffer << "/?";
                    else
                        buffer << "&";

                    buffer << key << "=" << value;
                    has_params = true;
                }
            }

            std::string to_string()
            {
                return buffer.str();
            }

    };

} // namespace tl

#endif