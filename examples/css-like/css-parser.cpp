#include <string>
#include <stack>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <tuple>

#include <tinyparser.hpp>

using namespace tipa;


/** example of css :

    button1[device=ipad] {
       font: "Roboto", 12
       textColor: "#FF0000"
       borderColor: "#000000"
    } 

    button1[device=iphone] {
       font: "Roboto", 12
       textColor: "#FF0000"
       borderColor: "#000000"
    } 
*/

int main(int argc, char *argv[])
{

    const token tk_hexacolor = create_lib_token("^#([0-9a-fA-F]{6})");
    
    // First the grammar
    // this is a hexadecimal color
    rule hexaColor = rule("\"") >> rule(tk_hexacolor) >> rule("\"");
    // this is a font
    rule font = rule("font") >> rule(":") >> rule("\"") >> rule(tk_ident) >> rule("\"") >> rule(",") >> rule(tk_int);
    // a textcolor
    rule textColor = rule("textColor") >> rule(":") >> hexaColor;
    // a border color
    rule borderColor = rule("borderColor") >> rule(":") >> hexaColor;    
    // a property is any of font, text, color (they are all optional)
    rule property = font | textColor | borderColor;

    // a button has a name (a identifier), and a device name (another identifier), and a sequence of properties.
    rule button = rule(tk_ident) >> rule('[') >> rule("device") >> rule("=")
                                 >> rule(tk_ident) >> rule(']') >> rule('{') >> *property >> rule('}');
    // the whole file is just a list of buttons
    rule root = *button;


    // now the data structures we are going to fill
    using FontS = std::pair<std::string, int>;
    struct ButtonS {
        std::string name;
        std::string device;
        FontS font;
        std::string textColor;
        std::string borderColor;
        
        void clear() { name = ""; device = ""; font.first = ""; font.second = 0; textColor = ""; borderColor = ""; }
    };

    std::vector<ButtonS> buttons;

    ButtonS temp;
    
    // now the parser actions
    // when the parser finds a font, store the values in the temp variable
    font.read_vars(temp.font.first, temp.font.second);
    // same for textColor
    textColor.read_vars(temp.textColor);
    // same for borderColor
    borderColor.read_vars(temp.borderColor);
    
    // when we detect a button, we store the temp variable in the vector of buttons
    button.set_action([&temp, &buttons](parser_context &pc) {
            std::vector<std::string> v;
            pc.collect_tokens(2, back_inserter(v));
            temp.name = v[0];
            temp.device = v[1];
            buttons.push_back(temp);
            temp.clear();
        });
    
    //----------------  end of parser specification -------------

    // An example of css file to parse
    std::stringstream sst(
        "button1[device=ipad] {\n"
        "font: \"Roboto\", 12\n"
        "textColor: \"#FF0000\"\n"
        "borderColor: \"#000000\"\n"
        "}\n" 
        "\n"
        "button1[device=iphone] {\n"
        "font: \"Roboto\", 12\n"
        "textColor: \"#FF0000\"\n"
        "borderColor: \"#000000\"\n"
        "}\n"
        );
    
    parser_context pc;
    pc.set_stream(sst);

    bool f = parse_all(root, pc);
    std::cout << "Parser status : " << std::boolalpha << f << std::endl;
    if (!f) {
        std::cout << pc.get_formatted_err_msg() << std::endl;
    }
    
    for (auto x : buttons) {
        std::cout << "-----------" << std::endl;
        std::cout << "Button : " << x.name << ", " << x.device << std::endl;
        std::cout << "font   : " << x.font.first << ", " << x.font.second << std::endl; 
        std::cout << "text   : " << x.textColor << std::endl;
        std::cout << "border : " << x.borderColor << std::endl;
    }    
}
