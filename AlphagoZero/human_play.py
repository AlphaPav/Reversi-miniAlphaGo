
from __future__ import print_function
from game import Game
from board import Board
from mcts_alphago import MCTSPlayer as MCTSPlayer_Alphago
from policy_value_net import PolicyValueNet


class Human(object):
    """
    human player
    """
    def __init__(self):
        self.player = None
        self.tag= "human"

    def set_player_ind(self, p):
        self.player = p

    def get_action(self, board):
        sensible_moves = board.get_all_avail_pos()
        try:
            if len(sensible_moves)==0:
                print("You have no place to go")
                return -1
            location = input("Your move: ")
            if isinstance(location, str):  # for python3
                location = [int(n, 10) for n in location.split(",")]
            move = board.location_to_move(location)

        except Exception as e:
            move = -1
        if move == -1 or move not in sensible_moves:
            print("invalid move")
            move = self.get_action(board)
        return move

    def __str__(self):
        return "Human {}".format(self.player)


def run():
    n = 5
    width, height = 8, 8
    model_file ='my_current_policy_51_14_28.model'
    try:
        board = Board(width=width, height=height, n_in_row=n)
        game = Game(board)

        # ############### human VS AI ###################
        # load the trained policy_value_net in either Theano/Lasagne, PyTorch or TensorFlow

        # best_policy = PolicyValueNet(width, height, model_file = model_file)
        # mcts_player = MCTSPlayer(best_policy.policy_value_fn, c_puct=5, n_playout=400)

        # load the provided model (trained in Theano/Lasagne) into a MCTS player written in pure numpy
        # try:
        #     policy_param = pickle.load(open(model_file, 'rb'))
        # except:
        #     policy_param = pickle.load(open(model_file, 'rb'),
        #                                encoding='bytes')  # To support python3
        best_policy = PolicyValueNet(width, height, model_file)
        mcts_player = MCTSPlayer_Alphago(best_policy.policy_value_fn,
                                 c_puct=5,
                                 n_playout=400)  # set larger n_playout for better performance

        # uncomment the following line to play with pure MCTS (it's much weaker even with a larger n_playout)
        # mcts_player = MCTS_Pure(c_puct=5, n_playout=1000)

        # human player, input your move in the format: 2,3
        human = Human()

        # set start_player=0 for human first
        game.start_play(human, mcts_player, start_player=1, is_shown=1)

    except KeyboardInterrupt:
        print('\n\rquit')

if __name__ == '__main__':
    run()
