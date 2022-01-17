/
#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <vector>
#include <random>
#include <QTimer>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Saves the x-coordinate to the attribute from the input
    void on_x_coordinate_editingFinished();
    // Moves, deletes and drops the fruits if possible
    void on_move_button_clicked();
    // Saves the y-coordinate to the attribute from the input
    void on_y_coordinate_editingFinished();
    void on_x_coordinate_returnPressed();
    void on_y_coordinate_returnPressed();
    // Drops all possible fruits on the board
    void drop_all_fruits();
    // Deletes the fruits from the board
    void delete_fruits();
    // Keeps clocks running
    void timer_slot();
    // Restarts the game and resets points and playtime
    void on_restart_button_clicked();

private:
    Ui::MainWindow *ui;
    // Timer for playtime
    QTimer* timer_;

    // Scene for the game grid
    QGraphicsScene* scene_;

    // Margins for the drawing area (the graphicsView object)
    // You can change the values as you wish
    const int TOP_MARGIN = 150;
    const int LEFT_MARGIN = 100;

    // Size of a square containing a fruit
    const int SQUARE_SIDE = 34;  // give your own value here
    // Number of vertical cells (places for fruits)
    const int ROWS = 10; // give your own value here
    // Number of horizontal cells (places for fruits)
    const int COLUMNS = 10; // give your own value here

    // Constants describing scene coordinates
    const int BORDER_UP = 0;
    const int BORDER_DOWN = SQUARE_SIDE * ROWS;
    const int BORDER_LEFT = 0;
    const int BORDER_RIGHT = SQUARE_SIDE * COLUMNS;

    const int NUMBER_OF_FRUITS = 5;
    // Defines the delay between moving fruits and dropping them
    const int DELAY = 3000;
    const QString LAST_COORDINATE = "J";
    const QString FIRST_COORDINATE = "A";

    // Defines all the possible colors for fruit, white represents empty space
    const std::vector<QBrush> possible_colors = {QBrush(Qt::red), QBrush(Qt::green), QBrush(Qt::yellow),
                                                QBrush(Qt::blue), QBrush(Qt::magenta), QBrush(Qt::white)};

    using game_board = std::vector<std::vector<QGraphicsRectItem*>>;
    // For randomly selecting fruits for the grid
    std::default_random_engine randomEng_;
    std::uniform_int_distribution<int> distr_;
    // The coordinates of the chosen fruit
    int first_x_;
    int first_y_;

    // Attribute to keep count of points
    int points_;

    // Attributes to track minutes and seconds
    int seconds_;
    int minutes_;

    // Contains all the fruits on the board
    game_board board_;

    // Generates the playing board in the beginning
    void generate_board();
    // Writes the titles for the grid rows and columns
    void init_titles();
    // Checks if it is possible to add given color to given coordinates
    bool color_is_allowed(const QBrush& color, const int& x_coord, const int& y_coord);
    // Swaps the location of two fruits
    void swap_fruits(const int& x_1, const int& y_1, const int& x_2, const int& y_2);
    // Checks if the fruit in the given coordinates is part of the match and adds the fruit to the vector if it is
    void fruit_is_part_of_match(const int& x_coord, const int& y_coord, std::vector<QGraphicsRectItem*>& fruits_to_delete);
    // Drops a single fruit in the given coordinates
    void drop_fruit(const int& x_coord, int y_coord);

};
#endif // MAINWINDOW_HH
