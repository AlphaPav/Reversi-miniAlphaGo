"""
A pure implementation of the Monte Carlo Tree Search (MCTS)

"""

import numpy as np
import copy
from operator import itemgetter


def rollout_policy_fn(board):
    """a coarse, fast version of policy_fn used in the rollout phase."""
    # rollout randomly
    available_moves = board.get_all_avail_pos() # a array of pos
    action_probs = np.random.rand(len(available_moves))
    return zip(available_moves, action_probs)


def policy_value_fn(board):
    """a function that takes in a state and outputs a list of (action, probability)
    tuples and a score for the state"""
    # return uniform probabilities and 0 score for pure MCTS
    available_moves = board.get_all_avail_pos()
    action_probs = np.ones(len(available_moves))/len(available_moves)
    return zip(available_moves, action_probs), 0


class TreeNode(object):
    """A node in the MCTS tree. Each node keeps track of its own value Q,
    prior probability P, and its visit-count-adjusted prior score u.
    """

    def __init__(self, parent, prior_p,current_player):
        self._parent = parent
        self._children = {}  # a map from action to TreeNode
        self._n_visits = 0
        self._Q = 0
        self._u = 0
        self._P = prior_p
        self._player =current_player
        self._tree_player = 2

    def is_leaf(self):
        """Check if leaf node (i.e. no nodes below this have been expanded).
        """
        return self._children == {}

    def is_root(self):
        return self._parent is None

    def expand(self,action_probs,current_player):
        for action, prob in action_probs:
            if action not in self._children:
                self._children[action] = TreeNode(self, prob,current_player)

    def get_value(self, c_puct):
        """Calculate and return the value for this node.
        It is a combination of leaf evaluations Q, and this node's prior
        adjusted for its visit count, u.
        c_puct: a number in (0, inf) controlling the relative impact of
            value Q, and prior probability P, on this node's score.
        """
        self._u = (c_puct * self._P *
                   np.sqrt(self._parent._n_visits) / (1 + self._n_visits))
        if self._player!=self._tree_player:
            return - self._Q + self._u
        return self._Q + self._u

    def select(self, c_puct):
        """Select action among children that gives maximum action value Q
        plus bonus u(P).
        Return: A tuple of (action, next_node)
        """
        # act_node[0] is the action
        # act_node[1] is the node
        return max(self._children.items(),
                   key=lambda act_node: act_node[1].get_value(c_puct)) # todo debug 调试一下这个是怎么运作的

    def update(self, leaf_value):
        """Update node values from leaf evaluation.
        leaf_value: the value of subtree evaluation from the current player's
            perspective.
        """
        # Count visit.
        self._n_visits += 1
        # Update Q, a running average of values for all visits.
        # n=n+1; w=w+v; Q=w/n;
        # therefore, Q = (Q*(n-1)+v)/n
        # therefore, Q = Q + (v-Q)/n
        self._Q += 1.0 * (leaf_value - self._Q) / self._n_visits


    def update_recursive(self, leaf_value):
        """Like a call to update(), but applied recursively for all ancestors.
        """
        # If it is not root, this node's parent should be updated first.
        if self._parent:
            # parent is another player, so have the negative "-leaf_value"
            self._parent.update_recursive(leaf_value)
        self.update(leaf_value)


class MCTS(object):
    """A simple implementation of Monte Carlo Tree Search."""

    def __init__(self, policy_value_fn, c_puct=5, n_playout=1600): # 10000
        self._root = TreeNode(None, 1.0, 1)
        self._policy = policy_value_fn
        self._c_puct = c_puct
        self._n_playout = n_playout

    def _playout(self, temp_board):
        """Run a single playout from the root to the leaf, getting a value at
        the leaf and propagating it back through its parents.
        State is modified in-place, so a copy must be provided.
        """
        node = self._root
        while(1):
            if node.is_leaf():
                break
            # Greedily select next move.
            action, node = node.select(self._c_puct) # select a child node as current node
            temp_board.do_move(action)

        if(len( temp_board.get_all_avail_pos())==0):
            return

        action_probs, _ = self._policy(temp_board)
        # Check for end of game
        end, winner = temp_board.game_end()
        if not end:
            node.expand(action_probs,temp_board.current_player) # add children for the current node
        # Evaluate the leaf node by random rollout;  play until the game is end
        leaf_value = self._evaluate_rollout(temp_board)  #  returning +1 if the current player wins
        # Update value and visit count of nodes in this traversal.
        node.update_recursive(leaf_value)

    def _evaluate_rollout(self, temp_board, limit=1000):
        """Use the rollout policy to play until the end of the game,
        returning +1 if the current player wins, -1 if the opponent wins,
        and 0 if it is a tie.
        """
        player = temp_board.get_current_player()
        winner=-1
        for i in range(limit): # play until the game is end
            end, winner = temp_board.game_end()
            if end:
                break
            if len(temp_board.get_all_avail_pos())==0:
                temp_board.skip_move()
                continue

            action_probs = rollout_policy_fn(temp_board)
            max_action = max(action_probs, key=itemgetter(1))[0] # todo debug 调试一下这个是怎么运作的
            temp_board.do_move(max_action)
        else:
            # If no break from the loop, issue a warning.
            print("WARNING: rollout reached move limit")
        if winner == -1:  # tie
            return 0
        else:
            return 1 if winner == player else -1


    def get_move(self, tempboard):
        """Runs all playouts sequentially and returns the most visited action.
        state: the current game state

        Return: the selected action
        """
        for n in range(self._n_playout): # 从根结点开始玩2000次, 最后选择最终的子节点
            tempboard_copy = copy.deepcopy(tempboard)
            self._playout(tempboard_copy)

        return max(self._root._children.items(),
                   key=lambda act_node: act_node[1]._n_visits)[0] # todo debug

    def update_with_move(self, last_move):
        """Step forward in the tree, keeping everything we already know
        about the subtree.
        """
        if last_move in self._root._children:
            self._root = self._root._children[last_move]
            self._root._parent = None
        else:
            self._root = TreeNode(None, 1.0,1)

    def __str__(self):
        return "MCTS"






class MCTSPlayer(object):
    """AI player based on MCTS"""
    def __init__(self, c_puct=5, n_playout=2000):
        self.mcts = MCTS(policy_value_fn, c_puct, n_playout)

    def set_player_ind(self, p):
        self.player = p

    def reset_player(self):
        self.mcts.update_with_move(-1)

    def get_action(self, board):
        sensible_moves = board.get_all_avail_pos()
        if len(sensible_moves) > 0:
            move = self.mcts.get_move(board)
            self.mcts.update_with_move(-1) # todo why it's  -1 ?
            return move
        else:
            print("WARNING: mcts_basic player no place to go")
            return -1

    def __str__(self):
        return "MCTS {}".format(self.player)



