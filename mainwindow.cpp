
#include "mainwindow.hh"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // We need a graphics scene in which to draw rectangles
    scene_ = new QGraphicsScene(this);

    // The width of the graphicsView is BORDER_RIGHT added by 2,
    // since the borders take one pixel on each side
    // (1 on the left, and 1 on the right).
    // Similarly, the height of the graphicsView is BORDER_DOWN added by 2.
    ui->graphicsView->setGeometry(LEFT_MARGIN, TOP_MARGIN,
                                  BORDER_RIGHT + 2, BORDER_DOWN + 2);
    ui->graphicsView->setScene(scene_);

    // The width of the scene_ is BORDER_RIGHT decreased by 1 and
    // the height of it is BORDER_DOWN decreased by 1, because
    // each square of a fruit is considered to be inside the sceneRect,
    // if its upper left corner is inside the sceneRect.
    scene_->setSceneRect(0, 0, BORDER_RIGHT - 1, BORDER_DOWN - 1);

    int seed = time(0); // You can change seed value for testing purposes
    randomEng_.seed(seed);
    distr_ = std::uniform_int_distribution<int>(0, NUMBER_OF_FRUITS - 1);
    distr_(randomEng_); // Wiping out the first random number (which is almost always 0)

    // Creates new timer, which updates time usage
    timer_ = new QTimer(this);
    connect(timer_, SIGNAL(timeout()), this, SLOT(timer_slot()));
    seconds_ = 0;
    minutes_ = 0;
    ui->lcd_minutes->setPalette(Qt::green);
    ui->lcd_seconds->setPalette(Qt::green);
    points_ = 0;
    ui->lcd_points->setPalette(Qt::blue);

    // Initializes all vector elements to nullptr to avoid crashing when initializing the board first time
    board_ = {{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
              {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
              {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
              {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
              {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
              {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
              {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
              {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
              {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
              {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
              };
    // Initializes the coordinates to be out of area
    first_x_ = -1;
    first_y_ = -1;
    init_titles();
    // Generates the starting board
    generate_board();
    // Starts the timer, timer sends signals once per second
    timer_->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;

}

void MainWindow::generate_board() {
    for (int row = 0; row < ROWS; ++row) {
        for (int column = 0; column < COLUMNS; ++column) {
            // Adds the rectangle to the scene
            QGraphicsRectItem* rect_to_add = scene_->addRect(column * SQUARE_SIDE, row * SQUARE_SIDE, SQUARE_SIDE, SQUARE_SIDE);
            QBrush color = possible_colors.at(distr_(randomEng_));
            // Generates new color if the current color can't be placed to grid
            while (not color_is_allowed(color, column, row)) {
                color = possible_colors.at(distr_(randomEng_));
            }
            rect_to_add->setBrush(color);
            // Saves the added rectangle to vector
            board_.at(row).at(column) = rect_to_add;
         }
    }
}

void MainWindow::init_titles() {
    // Displaying column titles, starting from A
    for(int i = 0, letter = 'A'; i < COLUMNS; ++i, ++letter)
    {
        int shift = 5;
        QString letter_string = "";
        letter_string.append(letter);
        QLabel* label = new QLabel(letter_string, this);
        label->setGeometry(LEFT_MARGIN + shift + i * SQUARE_SIDE,
                           TOP_MARGIN - SQUARE_SIDE,
                           SQUARE_SIDE, SQUARE_SIDE);
    }

    // Displaying row titles, starting from A
    for(int i = 0, letter = 'A'; i < ROWS; ++i, ++letter)
    {
        QString letter_string = "";
        letter_string.append(letter);
        QLabel* label = new QLabel(letter_string, this);
        label->setGeometry(LEFT_MARGIN - SQUARE_SIDE,
                           TOP_MARGIN + i * SQUARE_SIDE,
                           SQUARE_SIDE, SQUARE_SIDE);
    }
}

bool MainWindow::color_is_allowed(const QBrush& color, const int& x_coord, const int& y_coord) {
    // Checks the two horizontal neighbours of this rectangle
    if (x_coord > 1) {
        if (board_.at(y_coord).at(x_coord - 1)->brush() == color and
            board_.at(y_coord).at(x_coord - 2)->brush() == color) {
                return false;
        }
    }
    // Checks vertical neighbours
    if (y_coord > 1) {
        if (board_.at(y_coord - 1).at(x_coord)->brush() == color and
            board_.at(y_coord - 2).at(x_coord)->brush() == color) {
                return false;
        }
    }
    return true;
}
void MainWindow::swap_fruits(const int& x_1, const int& y_1, const int& x_2, const int& y_2) {
    QBrush first_color = board_.at(y_1).at(x_1)->brush();
    QBrush second_color = board_.at(y_2).at(x_2)->brush();
    // Swaps the color of the two fruits
    board_.at(y_1).at(x_1)->setBrush(second_color);
    board_.at(y_2).at(x_2)->setBrush(first_color);
    // Calls the delete function
    delete_fruits();
}

void MainWindow::delete_fruits() {
    // Vector, containing all the fruits to be deleted from the grid
    std::vector<QGraphicsRectItem*> fruits_to_delete;
    for (int row = 0; row < ROWS; ++row) {
        for (int column = 0; column < COLUMNS; ++column) {
            int x_coord = column;
            int y_coord = row;
            fruit_is_part_of_match(x_coord, y_coord, fruits_to_delete);
        }
    }
    // Changes the color of deleted fruits to white
    for (auto rect : fruits_to_delete) {
        rect->setBrush(Qt::white);
    }
    // Adds points for the player
    points_ += fruits_to_delete.size();
    ui->lcd_points->display(points_);

    // Drops fruits if fruits were deleted
    if (fruits_to_delete.size() != 0) {
    // Waits before dropping the fruits
    QTimer::singleShot(DELAY, this, SLOT(drop_all_fruits()));
    }
    else {
        ui->move_button->setEnabled(true);
        ui->restart_button->setEnabled(true);
    }
}

void MainWindow::timer_slot() {
    seconds_ += 1;
    if (seconds_ == 60) {
        minutes_ += 1;
        seconds_ = 0;
    }
    // Updates the time
    ui->lcd_minutes->display(minutes_);
    ui->lcd_seconds->display(seconds_);
}

void MainWindow::fruit_is_part_of_match(const int& x_coord, const int& y_coord, std::vector<QGraphicsRectItem*>& fruits_to_delete) {
    QBrush color = board_.at(y_coord).at(x_coord)->brush();
    // If color is white, then the coordinates contains empty space
    if (color == Qt::white) {
        return;
    }
    // Checks the two rectangles below given coordinates
    if (y_coord + 2 < ROWS) {
        if (board_.at(y_coord + 1).at(x_coord)->brush() == color and
                board_.at(y_coord + 2).at(x_coord)->brush() == color) {
            fruits_to_delete.push_back(board_.at(y_coord).at(x_coord));
            return;
        }

    }
    // Checks the two rectangles right to the given coordinates
     if (x_coord + 2 < COLUMNS) {
        if (board_.at(y_coord).at(x_coord + 1)->brush() == color and
                board_.at(y_coord).at(x_coord + 2)->brush() == color) {
            fruits_to_delete.push_back(board_.at(y_coord).at(x_coord));
            return;

        }

    }
     // Checks the two rectangles left to the given coordinates
     if (y_coord - 2 >= 0) {
        if (board_.at(y_coord - 1).at(x_coord)->brush() == color and
                board_.at(y_coord - 2).at(x_coord)->brush() == color) {
            fruits_to_delete.push_back(board_.at(y_coord).at(x_coord));
            return;
    }

    }
    // Checks the coordinates above the given coordinates
    if (x_coord - 2 >= 0) {
        if (board_.at(y_coord).at(x_coord - 1)->brush() == color and
                board_.at(y_coord).at(x_coord - 2)->brush() == color) {
            fruits_to_delete.push_back(board_.at(y_coord).at(x_coord));
            return;
        }

    }
    // Checks if the given coordinates is in the middle of match
    if (0 <= x_coord - 1 and x_coord + 1 < COLUMNS) {
        if (board_.at(y_coord).at(x_coord - 1)->brush() == color and
                board_.at(y_coord).at(x_coord + 1)->brush() == color) {
            fruits_to_delete.push_back(board_.at(y_coord).at(x_coord));
            return;
        }
    }
    if (0 <= y_coord - 1 and y_coord + 1 < ROWS) {
        if (board_.at(y_coord - 1).at(x_coord)->brush() == color and
                board_.at(y_coord + 1).at(x_coord)->brush() == color) {
            fruits_to_delete.push_back(board_.at(y_coord).at(x_coord));
            return;
        }
    }
}

void MainWindow::drop_fruit(const int &x_coord, int y_coord) {
    // Drops the fruit while it is not in the bottom and there is no fruit below
    while (y_coord != ROWS - 1 and board_.at(y_coord + 1).at(x_coord)->brush() == Qt::white) {
        QBrush current_color = board_.at(y_coord).at(x_coord)->brush();
        board_.at(y_coord).at(x_coord)->setBrush(Qt::white);
        board_.at(y_coord + 1).at(x_coord)->setBrush(current_color);
        ++y_coord;
    }
}

void MainWindow::drop_all_fruits() {
    // Calls drop_fruit function for every fruit on the board.
    for (int row = ROWS - 2; row >= 0; --row) {
        for (int column = 0; column < COLUMNS; ++column) {
            drop_fruit(column, row);
        }
    }
    QTimer::singleShot(DELAY / 2, this, SLOT(delete_fruits()));
}

void MainWindow::on_x_coordinate_editingFinished() {
    QString coordinate = ui->x_coordinate->text().toUpper();

    if (coordinate.size() == 1 and FIRST_COORDINATE <= coordinate and coordinate <= LAST_COORDINATE) {
        // Converts the ASCIi number of coordinate to real coordinate
        first_x_ = coordinate.at(0).toLatin1() - 65;
    }
    else {
    first_x_ = -1;
    }
}

void MainWindow::on_move_button_clicked() {
    ui->moveResult->clear();
    // Checks that the given coordinates belong to the grid
    if (0 <= first_x_ and first_x_ < COLUMNS and 0 <= first_y_ and first_y_ < ROWS) {
        // Checks whether the swap is horizontal or vertical
        if (ui->horizontalCheckbox->isChecked()) {
            // Checks that there is fruit next to the given coordinates
            if (first_x_ + 1 < COLUMNS and board_.at(first_y_).at(first_x_)->brush() != Qt::white and
                    board_.at(first_y_).at(first_x_ + 1)->brush() != Qt::white) {
                ui->move_button->setEnabled(false);
                ui->restart_button->setEnabled(false);
                ui->moveResult->setPlainText("Swap was successfull!");
                swap_fruits(first_x_, first_y_, first_x_ + 1, first_y_);
            }
            else {
                ui->moveResult->setPlainText("Invalid input.");
            }
        }
        else {
            if (first_y_ + 1 < ROWS and board_.at(first_y_).at(first_x_)->brush() != Qt::white and
                    board_.at(first_y_ + 1).at(first_x_)->brush() != Qt::white) {
                ui->move_button->setEnabled(false);
                ui->restart_button->setEnabled(false);
                ui->moveResult->setPlainText("Swap was successfull!");
                swap_fruits(first_x_, first_y_, first_x_, first_y_ + 1);
            }
            else {
                ui->moveResult->setPlainText("Invalid input.");
            }
        }
    }
    else {
        ui->moveResult->setPlainText("Invalid input.");
    }
}

void MainWindow::on_y_coordinate_editingFinished() {
    QString coordinate = ui->y_coordinate->text().toUpper();
    // Checks that the coordinate is between first and last coordinates
    if (coordinate.size() == 1 and FIRST_COORDINATE <= coordinate and coordinate <= LAST_COORDINATE) {
        // Converts the ASCIi number of coordinate to real coordinate
        first_y_ = coordinate.at(0).toLatin1() - 65;
    }
    else {
    first_y_ = -1;
    }
}

void MainWindow::on_x_coordinate_returnPressed() {
    on_x_coordinate_editingFinished();
}

void MainWindow::on_y_coordinate_returnPressed() {
    on_y_coordinate_editingFinished();
}

void MainWindow::on_restart_button_clicked() {
    points_ = 0;
    seconds_ = 0;
    minutes_ = 0;
    ui->lcd_seconds->display(seconds_);
    ui->lcd_minutes->display(minutes_);
    ui->lcd_points->display(points_);
    ui->moveResult->clear();
    ui->x_coordinate->clear();
    ui->y_coordinate->clear();
    first_x_ = -1;
    first_y_ = -1;
    generate_board();
}
