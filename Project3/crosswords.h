#ifndef CROSSWORDS_H
#define CROSSWORDS_H

#include <ostream>
#include <compare>
#include <initializer_list>
#include <unordered_map>
#include <set>
#include <vector>
#include <cstddef>
#include <cassert>

using std::pair;
using std::string;
using std::move;
using std::copy;
using std::min;
using std::max;
using std::unordered_map;
using std::vector;
using std::set;
using std::initializer_list;
using std::hash;
using coordinate_t = size_t;
using length_t = size_t;
using dim_t = pair<length_t, length_t>;
using pos_t = pair<coordinate_t, coordinate_t>;
using orientation_t = enum orientation {H, V};
using letter_id = pair<char, orientation>;

constexpr int EXTRA_CORNERS = 2;
constexpr int NEXT_DOT = 2;
constexpr int NULL_IN_ASCII = 0;
constexpr int LITTLE_A_IN_ASCII = 65;
constexpr int LITTLE_Z_IN_ASCII = 91;
constexpr int BIG_A_IN_ASCII = 97;
constexpr int BIG_Z_IN_ASCII = 122;
constexpr int DIFFRENCE_BETWEEN_BIG_AND_SMALL_LETTER = 32;
constexpr int PROTECTED_CHAR = '.';

inline string DEFAULT_WORD = "?";
inline char DEFAULT_CHAR = '\0';
inline char CROSSWORD_BACKGROUND = '.';

class RectArea {
    public:
        // Default constructor
        RectArea(pos_t arg1, pos_t arg2) : 
                left_top(arg1), right_bottom(arg2) {}
        
        // Copy constructor
        RectArea(const RectArea& other) = default;
        
        // Copy operator
        RectArea& operator=(const RectArea& other) {
            if (this != &other) {
                left_top = other.left_top;
                right_bottom = other.right_bottom;
            }
            return *this;
        }

        // Move constructor
        RectArea(RectArea&& other) = default;

        // Move operator
        RectArea& operator=(RectArea&& other) noexcept {
            if (this != &other) {
                left_top = move(other.left_top);
                right_bottom = move(other.right_bottom);
            }

            return *this;
        }

        // Getters, which share information abut RectArea's size
        pos_t get_left_top() const {
            return left_top;
        }

        pos_t get_right_bottom() const {
            return right_bottom;
        }

        void set_left_top(pos_t new_left_top) {
            left_top = new_left_top;
        }

        void set_right_bottom(pos_t new_right_bottom) {
            right_bottom = new_right_bottom;
        }
        
        // Function determines if RectArea is empty
        bool empty() const {
           if (right_bottom.first >= left_top.first && 
                right_bottom.second >= left_top.second)
                return false;

            return true;
        }

        // Function returns maximum area occupied by RectArea
        dim_t size() const {
            length_t width = 0;
            length_t height = 0;

            if (!empty()) {
                width = right_bottom.first - left_top.first + 1;
                height = right_bottom.second - left_top.second + 1;
            }
            assert(left_top.first != 0 || right_bottom.first != SIZE_MAX);
            assert(left_top.second != 0 || right_bottom.second != SIZE_MAX);
            return {width, height};
        }

        // Function extends the area by a given point
        void embrace(pos_t new_point) {
            pos_t new_point_left = new_point;
            pos_t new_point_right = new_point;

            if (!empty()) {            
                new_point_left.first = min(left_top.first, 
                                        new_point.first);
                new_point_left.second = min(left_top.second, 
                                        new_point.second);
                new_point_right.first = max(right_bottom.first,
                                        new_point.first);
                new_point_right.second = max(right_bottom.second, 
                                        new_point.second);
            }

            left_top = new_point_left;
            right_bottom = new_point_right;
        }

        // Operators combine two areas
        RectArea operator*(const RectArea& other) const {
            if (empty() || other.empty())
                return RectArea(pos_t(1,1), pos_t(0,0));

            pos_t new_left_top;
            pos_t new_right_bottom;
            new_left_top.first = max(this->left_top.first, 
                                    other.left_top.first);
            new_left_top.second = max(this->left_top.second,
                                    other.left_top.second);
            new_right_bottom.first = min(this->right_bottom.first,
                                    other.right_bottom.first);
            new_right_bottom.second = min(this->right_bottom.second,
                                    other.right_bottom.second);

            return RectArea(new_left_top, new_right_bottom);
        }

        RectArea& operator*=(const RectArea& other) {
            *this = *this * other;
            return *this;
        }

    private:
        pos_t left_top;
        pos_t right_bottom;
};

inline RectArea DEFAULT_EMPTY_RECT_AREA = RectArea(pos_t(1,1), pos_t(0,0));

class Word {
    private:
        pos_t coordinates;
        orientation_t orientation;
        string word;

    public:
        // Default constructor
        Word(coordinate_t first, coordinate_t second, 
            orientation_t o, string new_word)
            : coordinates(pos_t(first,second)), orientation(o), 
                word(new_word) {
            if (new_word.empty()) {
                this->word = DEFAULT_WORD;
            }

            if (o == H && SIZE_MAX - first < new_word.length() + 1) {
                this->word = new_word.substr(0, (SIZE_MAX - first) + 1);
            }

            if (o == V && SIZE_MAX - first < new_word.length() + 1) {
                this->word = new_word.substr(0, (SIZE_MAX - second) + 1);
            }
        }

        // Copy constructor
        Word(const Word& other) = default;

        // Copy operator
        Word& operator=(const Word& other) {
            if (this != &other) {
                coordinates = other.coordinates;
                orientation = other.orientation;
                word = other.word;
            }
            return *this;
        }

        // Move constructor
        Word(Word&& other) = default;

        // Move operator
        Word& operator=(Word&& other) noexcept {
            if (this != &other) {
                coordinates = other.coordinates;
                orientation = other.orientation;
                word = other.word;
            }
            return *this;
        }

        // Equal operator
        bool operator==(const Word& other) const {
            if (coordinates == other.coordinates && 
                orientation == other.orientation) {
                return true;
            }
            return false;
        };

        // Compare operator
        auto operator<=>(const Word& other) const {
            if(coordinates.first < other.coordinates.first) {
                return std::strong_ordering::less;
            }

            else if(coordinates.first > other.coordinates.first) {
                return std::strong_ordering::greater;
            }

            else if(coordinates.second < other.coordinates.second) {
                return std::strong_ordering::less;
            }

            else if(coordinates.second > other.coordinates.second) {
                return std::strong_ordering::greater;
            }

            else if(orientation == H && other.orientation == V) {
                return std::strong_ordering::less;
            }

            else if(orientation == V && other.orientation == H) {
                return std::strong_ordering::greater;
            }

            else {
                return std::strong_ordering::equal;
            }
        }

        // Getters, which share information about Word
        pos_t get_start_position() const {
            return coordinates;
        }

        pos_t get_end_position() const {
            if (orientation == H) {
                coordinate_t new_position = coordinates.first;
                if (SIZE_MAX - new_position < this->word.length()) {
                    return pos_t(SIZE_MAX, coordinates.second);
                }
                new_position+=word.length() - 1;
                coordinate_t old_position = coordinates.second;
                pos_t end_position(new_position, old_position);
                return end_position;
            }

            else {
                coordinate_t new_position = coordinates.second;
                if (SIZE_MAX - new_position < this->word.length()) {
                    return pos_t(coordinates.first, SIZE_MAX);
                }
                new_position+=word.length() - 1;
                coordinate_t old_position = coordinates.first;
                pos_t end_position(old_position, new_position);
                return end_position;
            }
        }

        orientation_t get_orientation() const {
            return orientation;
        }

        char at(coordinate_t letter) const {
            if ((word == "?" && letter == 0) ||
                letter >= this->word.length()) {
                return DEFAULT_CHAR;
            }
            return word[letter];
        }

        coordinate_t length() const {
            return word.length();
        }

        //Function returns part of Crossword, occupied by Word
        RectArea rect_area() const {
            return RectArea((*this).get_start_position(), 
                            (*this).get_end_position());
        }
};

using words = vector<Word>;

class Crossword {
    private:

        //Hash function for our type of map
        struct hash_fun {
            coordinate_t operator()(const pos_t &pos) const {
                return hash<coordinate_t>()(pos.first) ^ 
                        (hash<coordinate_t>()(pos.second) << 1);
            }
        };
        using letters = unordered_map<pos_t, letter_id, hash_fun>;

        RectArea area;
        coordinate_t how_many_vertically;
        coordinate_t how_many_horizontally;
        words Words;
        letters crossword_letters;

        // Auxiliary function, determines if given word can be inserted
        // vertically
        bool check_word_vertically(Word word, pos_t start_position, 
                                   pos_t end_position) {
        coordinate_t start = start_position.second;
        coordinate_t end = end_position.second;
        coordinate_t col = start_position.first;
        coordinate_t row = start;
        while (row <= end) {
            auto it = crossword_letters.find(pos_t(col, row)); 
            if (it != crossword_letters.end()) {
                if (it->second.first == '.') {
                    if (row == SIZE_MAX || word.length() == 1) {
                        return false;
                    }
                    

                    auto it2 = crossword_letters.find(pos_t(col, row+1));
                    if (it2 != crossword_letters.end()) {
                        if (it2->second.first == PROTECTED_CHAR ||
                            it2->second.first != 
                            convert_letter(word.at(row+1 - start))) {
                            return false;
                        }

                        else { 
                            if (col > 0) {
                                auto it3 = crossword_letters.find(
                                            pos_t(col-1, row));
                                auto it4 = crossword_letters.find(
                                            pos_t(col-1, row+NEXT_DOT));
                                if (it3 != crossword_letters.end() &&
                                    it3->second.first != PROTECTED_CHAR) {
                                        return false;
                                }

                                if (it4 != crossword_letters.end() &&
                                    it4->second.first != PROTECTED_CHAR) {
                                        return false;
                                }
                            }

                            if (col < SIZE_MAX) {
                                auto it5 = crossword_letters.find(
                                            pos_t(col+1, row));
                                auto it6 = crossword_letters.find(
                                            pos_t(col+1, row+ NEXT_DOT));
                                if (it5 != crossword_letters.end() &&
                                    it5->second.first != PROTECTED_CHAR) {
                                        return false;
                                }

                                if (it6 != crossword_letters.end() &&
                                    it6->second.first != PROTECTED_CHAR) {
                                        return false;
                                }
                            }
                        }
                    } 
                }
                else if (it->second.second == V || 
                    it->second.first != convert_letter(word.at(row - start))) {
                        return false;
                    }
                }
                if (row == SIZE_MAX)
                    break;
                row++;
            }

            return true;
        }

        // Auxiliary function, determines if given word can be inserted
        // horizontally
        bool check_word_horizontally(Word word, pos_t start_position, 
                                     pos_t end_position) {
            coordinate_t start = start_position.first;
            coordinate_t end = end_position.first;
            coordinate_t row = start_position.second;

            coordinate_t col = start;
            while (col <= end) {
                auto it = crossword_letters.find(pos_t(col, row));
                if (it != crossword_letters.end()) {
                    if (it->second.first == PROTECTED_CHAR) {
                        if (col == SIZE_MAX || word.length() == 1) {
                            return false;
                        }
                        

                        auto it2 = crossword_letters.find(pos_t(col+1, row));
                        if (it2 != crossword_letters.end()) {
                            if (it2->second.first == PROTECTED_CHAR ||
                                it2->second.first != 
                                convert_letter(word.at(col+1 - start))) {
                                return false;
                            }

                            else { 
                                if (row > 0) {
                                    auto it3 = crossword_letters.find(
                                                pos_t(col, row-1));
                                    auto it4 = crossword_letters.find(
                                                pos_t(col+NEXT_DOT, row-1));

                                    if (it3 != crossword_letters.end() &&
                                        it3->second.first != PROTECTED_CHAR) {
                                            return false;
                                    }

                                    if (it4 != crossword_letters.end() &&
                                        it4->second.first != PROTECTED_CHAR) {
                                            return false;
                                    }
                                }

                                if (row < SIZE_MAX) {
                                    auto it5 = crossword_letters.find(
                                                pos_t(col, row+1));
                                    auto it6 = crossword_letters.find(
                                                pos_t(col+NEXT_DOT, row+1));

                                    if (it5 != crossword_letters.end() &&
                                        it5->second.first != PROTECTED_CHAR) {
                                            return false;
                                    }

                                    if (it6 != crossword_letters.end() &&
                                        it6->second.first != PROTECTED_CHAR) {
                                            return false;
                                    }
                                }

                            }
                        } 
                    }

                    else {
                        if (it->second.second == H || 
                            it->second.first != 
                            convert_letter(word.at(col - start))) {
                            return false;
                        }
                    }
                }
                if (col == SIZE_MAX)
                    break;
                col++;
            }
            return true;
        }
            
        // Function checks, if given word, can be inserted into Crossword
        bool checkWord(Word word, orientation o) {
            pos_t start_position = word.get_start_position();
            pos_t end_position = word.get_end_position();
            if (o == V) {
                return check_word_vertically(word, start_position, 
                                                end_position);
            }

            return check_word_horizontally(word, start_position, 
                                            end_position);
        }

        // Function converts given char to other, that can be insterted into
        // Crossword
        char convert_letter(char c) {
            if(c >= BIG_A_IN_ASCII && c <= BIG_Z_IN_ASCII) {
                return c - DIFFRENCE_BETWEEN_BIG_AND_SMALL_LETTER;
            }
            else if((c >= NULL_IN_ASCII && c < LITTLE_A_IN_ASCII) || 
            (c > LITTLE_Z_IN_ASCII && c < BIG_A_IN_ASCII) 
            || c > BIG_Z_IN_ASCII) {
                return '?';
            }
            return c;
        }

        // Auxiliary function, which inserts vertically words
        void insert_word_vertically(Word word, pos_t end_position, 
                                    coordinate_t row, coordinate_t column) {
            coordinate_t i = 0;
            (this->how_many_vertically)++;
            coordinate_t j = row;
            while (j <= end_position.second) {
                crossword_letters[pos_t(column, j)] = 
                letter_id(convert_letter(word.at(i)), V);
                i++;
                if (j == SIZE_MAX)
                        break;
                j++; 
            }

            if (column > 0) {
                j = row;
                while (j <= end_position.second) {
                    crossword_letters.insert({pos_t(column - 1, j),
                                            letter_id(PROTECTED_CHAR, V)});
                if (j == SIZE_MAX)
                    break;
                j++; 
                }
            }

            if (column < SIZE_MAX) {
                j = row;
                while (j <= end_position.second) {
                    crossword_letters.insert({pos_t(column + 1, j), 
                                            letter_id(PROTECTED_CHAR, V)});
                if (j == SIZE_MAX)
                        break;
                j++; 
                }
            }

            if (row > 0) {
                if (column > 0) {
                    crossword_letters.insert({pos_t(column - 1, row - 1),
                                            letter_id(PROTECTED_CHAR, V)});
                }

                if (column < SIZE_MAX) {
                    crossword_letters.insert({pos_t(column + 1, row - 1),
                                            letter_id(PROTECTED_CHAR, V)});
                }

                crossword_letters.insert({pos_t(column, row - 1),
                                        letter_id(PROTECTED_CHAR, V)});
            }

            if (end_position.second < SIZE_MAX) {
                if (column > 0) {
                    crossword_letters.insert(
                        {pos_t(column - 1, end_position.second + 1), 
                        letter_id(PROTECTED_CHAR, V)});
                }

                if (column < SIZE_MAX) {
                    crossword_letters.insert(
                        {pos_t(column + 1, end_position.second + 1),
                        letter_id(PROTECTED_CHAR, V)});
                }

                crossword_letters.insert(
                    {pos_t(column, end_position.second + 1),
                    letter_id(PROTECTED_CHAR, V)});
            }
        }

        // Auxiliary function, which inserts horizontally words
        void insert_word_horizontally(Word word, pos_t end_position,  
                                      coordinate_t row, coordinate_t column) {
            coordinate_t i = 0;
            (this->how_many_horizontally)++;
            coordinate_t j = column;
            while (j <= end_position.first) {
                crossword_letters[pos_t(j, row)] = 
                letter_id(convert_letter(word.at(i)), H);
                i++;
                if (j == SIZE_MAX)
                    break;
                j++;
            }

            if (row > 0) {
                j = column;
                while (j <= end_position.first) {
                    crossword_letters.insert({pos_t(j, row - 1), 
                                            letter_id(PROTECTED_CHAR, H)});
                    if (j == SIZE_MAX)
                        break;
                    j++;                       
                }
            }

            if (row < SIZE_MAX) {
                j = column;
                while(j <= end_position.first) {
                    crossword_letters.insert
                    ({pos_t(j, row + 1), letter_id(PROTECTED_CHAR, H)});
                    if (j == SIZE_MAX)
                        break;
                    j++; 
                }
            }

            if (column > 0) {
                if (row > 0) {
                    crossword_letters.insert
                    ({pos_t(column-1, row-1), letter_id(PROTECTED_CHAR, H)});
                }

                if (row < SIZE_MAX) {
                    crossword_letters.insert
                    ({pos_t(column-1, row+1), letter_id(PROTECTED_CHAR, H)});
                }

                crossword_letters.insert({pos_t(column-1, row), 
                                        letter_id(PROTECTED_CHAR, H)});
            }

            if (end_position.first < SIZE_MAX) {
                if (row > 0) {
                    crossword_letters.insert(
                        {pos_t(end_position.first + 1, row - 1),
                        letter_id(PROTECTED_CHAR, H)});
                }

                if (row < SIZE_MAX) {
                    crossword_letters.insert(
                        {pos_t(end_position.first + 1, row + 1),
                        letter_id(PROTECTED_CHAR, H)});
                }

                crossword_letters.insert(
                    {pos_t(end_position.first + 1, row),
                    letter_id(PROTECTED_CHAR, H)});
            }   
        }

    public:
    
        //Funkcja dodaje słowa do krzyżówki
        bool insert_word(Word word) {
            if (!(checkWord(word, word.get_orientation()))) {
                return false;
            }

            this->Words.push_back(word);
            this->area.embrace(word.get_start_position());
            this->area.embrace(word.get_end_position());
            pos_t start_position = word.get_start_position();
            coordinate_t column = start_position.first;
            coordinate_t row = start_position.second;
            pos_t end_position = word.get_end_position();

            if(word.get_orientation() == V) {
                insert_word_vertically(word, end_position, 
                                    row, column);
            }

            else {
                insert_word_horizontally(word, end_position, 
                                        row, column);
            }

            return true;
        }

        // Default constructor
        Crossword(Word firstWord, initializer_list<Word> otherWords) 
                : area(DEFAULT_EMPTY_RECT_AREA), 
                how_many_vertically(0), how_many_horizontally(0) {
            insert_word(firstWord);

            for (auto const & word: otherWords) {
                insert_word(word);
            }
            
        }

        // Copy constructor
        Crossword(const Crossword& other):
                area(other.area),
                how_many_vertically(other.how_many_vertically),
                how_many_horizontally(other.how_many_horizontally),
                Words(other.Words),
                crossword_letters(other.crossword_letters) {}
                  
        // Copy operator
        Crossword& operator=(const Crossword & other) {
            if (this != &other) {
                how_many_horizontally = other.how_many_horizontally;
                how_many_vertically = other.how_many_vertically;
                Words = other.Words;
                crossword_letters = other.crossword_letters;
                area = other.area;
            }

            return *this;
        }

        // Move constructor
        Crossword(Crossword&& other) noexcept : 
                area(move(other.area)),
                how_many_vertically(move(other.how_many_vertically)),
                how_many_horizontally(move(other.how_many_horizontally)), 
                Words(move(other.Words)),
                crossword_letters(move(other.crossword_letters)) {

                    other.area = DEFAULT_EMPTY_RECT_AREA;
                    other.how_many_vertically = 0;
                    other.how_many_horizontally = 0;
                    other.Words.clear();
                    other.crossword_letters.clear();
        }             

        // Move operator
        Crossword& operator=(Crossword && other) noexcept {
            if (this != &other) {
                area = move(other.area);
                how_many_vertically = move(other.how_many_vertically);
                how_many_horizontally = move(other.how_many_horizontally);
                Words = move(other.Words);
                crossword_letters = move(other.crossword_letters);
            }
        
            return *this;
        }

        // Operator, which enables printing Crossword.
        friend std::ostream & operator<<
        (std::ostream & os, Crossword & crossword) {
            for (coordinate_t col = crossword.area.get_left_top().first; 
                col < crossword.area.get_right_bottom().first + EXTRA_CORNERS;
                col++) {
                        os << CROSSWORD_BACKGROUND << " ";
            }

            os << CROSSWORD_BACKGROUND << '\n';

            for (coordinate_t row = crossword.area.get_left_top().second; 
                row <= crossword.area.get_right_bottom().second; row++) {
                os << CROSSWORD_BACKGROUND << " ";

                for (coordinate_t col = crossword.area.get_left_top().first; 
                    col <= crossword.area.get_right_bottom().first; col++) {
                        auto it = crossword.crossword_letters
                        .find(pos_t(col, row));

                        if (it != crossword.crossword_letters.end() &&
                                it->second.first != PROTECTED_CHAR) {
                                os << it->second.first << " ";
                        }
                            
                        else {
                            os << CROSSWORD_BACKGROUND << " ";
                        }
                        
                    }
                os << CROSSWORD_BACKGROUND << '\n';
            }

            for (coordinate_t col = crossword.area.get_left_top().first; 
                col < crossword.area.get_right_bottom().first + EXTRA_CORNERS;
                col++) {
                        os << CROSSWORD_BACKGROUND << " ";
            }
            os << CROSSWORD_BACKGROUND << '\n';
            return os;
        }       

        //Overloaded operators, which enable adding two Crosswords
        Crossword operator+(Crossword& other) {
            Crossword cr = *this;
            for (auto  & word : other.Words) {
                if(cr.checkWord(word, word.get_orientation())) {
                    cr.insert_word(word);
                }
            }
            return cr;
        }
            
        Crossword & operator+=(Crossword& other) {
            *this = *this + other;
            return *this;
        }


        // Function calculates the amount of words oriented horizontally
        // and vertically
        pos_t word_count() {
            return pos_t(how_many_horizontally, how_many_vertically);
        }

        // Function returns the amount of signs oriented horizontally
        // and vertically. 
        pos_t size() {
            coordinate_t width = 0;
            coordinate_t height = 0;

            if (!area.empty()) {
                width = area.get_right_bottom().first 
                        - area.get_left_top().first + 1;
                height = area.get_right_bottom().second 
                        - area.get_left_top().second + 1;
            }

            return {width, height};
        }
};

#endif
