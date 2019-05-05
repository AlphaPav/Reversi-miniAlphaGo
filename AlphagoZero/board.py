import  numpy as np
class Pos_t:
    def __init__(self):
        self.x= -1
        self.y= -1

class Board(object):
    """board for the game"""
    def __init__(self, **kwargs):
        self.width =int(kwargs.get('width', 8))
        self.height =int(kwargs.get('height', 8))
        # board states stored as a dict,
        # key: move as location on the board,
        # value: player as pieces type
        self.states = {}
        self.players = [1, 2]  # player1 and player2

    def init_board(self, start_player=0):

        self.current_player = self.players[start_player]  # start player
        self.next_player=(
            self.players[0] if self.current_player == self.players[1]
            else self.players[1]
        )
        # keep available moves in a list
        self.availables = list(range(self.width * self.height))
        self.states = {}
        self.last_move = -1
        self.states[self.location_to_move([3, 3])] = self.current_player
        self.states[self.location_to_move([4, 4])] = self.current_player
        self.states[self.location_to_move([3, 4])] = self.next_player
        self.states[self.location_to_move([4, 3])] = self.next_player
        self.availables.remove(self.location_to_move([3, 3]))
        self.availables.remove(self.location_to_move([4, 4]))
        self.availables.remove(self.location_to_move([3, 4]))
        self.availables.remove(self.location_to_move([4, 3]))


    def move_to_location(self, move):
        """
        3*3 board's moves like:
        6 7 8
        3 4 5
        0 1 2
        and move 5's location is (1,2)
        """
        h = move // self.width
        w = move % self.width
        return [h, w]

    def location_to_move(self, location):
        if len(location) != 2:
            return -1
        h = location[0]
        w = location[1]
        move = h * self.width + w
        if move not in range(self.width * self.height):
            return -1
        return move

    def current_state(self):
        """return the board state from the perspective of the current player.
        state shape: 4*width*height
        """

        square_state = np.zeros((4, self.width, self.height))
        if self.states:
            moves, players = np.array(list(zip(*self.states.items())))
            move_curr = moves[players == self.current_player]
            move_oppo = moves[players != self.current_player]
            square_state[0][move_curr // self.width,
                            move_curr % self.height] = 1.0 # 当前player走过的棋子
            square_state[1][move_oppo // self.width,
                            move_oppo % self.height] = 1.0 # 对手player走过的棋子
            # indicate the last move location
            square_state[2][self.last_move // self.width,
                            self.last_move % self.height] = 1.0 # 上一步棋下的位置
        if len(self.states) % 2 == 0: # todo 如果是2 号player ？ 有啥意义？
            square_state[3][:, :] = 1.0  # indicate the colour to play
        return square_state[:, ::-1, :]#todo 检查返回值数组的形状


    def skip_move(self):
        self.next_player = self.current_player
        self.current_player = (
            self.players[0] if self.current_player == self.players[1]
            else self.players[1]
        )

    def do_move(self, move):

        reversi_pos= self.get_reversi_pos(move)
        if len(reversi_pos)==0:
            print("unexpected error in reversi") # todo : totest the case
        for  i in range(0, len(reversi_pos)):
            self.states[reversi_pos[i]]= self.current_player

        self.states[move] = self.current_player
        self.availables.remove(move)

        self.next_player= self.current_player
        self.current_player = (
            self.players[0] if self.current_player == self.players[1]
            else self.players[1]
        )
        self.last_move = move

    def get_reversi_pos(self,move):
        location= self.move_to_location(move)
        h = location[0]
        w = location[1]
        available_direction=[]
        for i in range(0, 3):
            for j in range(0, 3):
                temp_h = (location[0] - 1 + i)
                temp_w = (location[1] - 1 + j)
                temp_move = self.location_to_move([temp_h, temp_w])
                if temp_move not in self.states:  # empty
                    continue
                elif self.states[temp_move] == self.current_player:
                    continue
                else:  # another player
                    delta_h = temp_h - h
                    delta_w = temp_w - w
                    while self.board_check(temp_h + delta_h) and self.board_check(temp_w + delta_w):
                        temp_h += delta_h
                        temp_w += delta_w
                        temp_move = self.location_to_move([temp_h, temp_w])
                        if temp_move not in self.states:
                            break
                        elif self.states[temp_move] == self.current_player:
                            available_direction.append([i,j])
        reversi_pos = []
        for k in range (0,len(available_direction)):
            i,j= available_direction[k]
            temp_h= (location[0]-1+i)
            temp_w= (location[1]-1+j)
            temp_move= self.location_to_move([temp_h,temp_w])
            reversi_pos.append(temp_move)

            delta_h = temp_h - h
            delta_w = temp_w - w
            while self.board_check(temp_h+delta_h) and self.board_check(temp_w+delta_w):
                temp_h += delta_h
                temp_w += delta_w
                temp_move= self.location_to_move([temp_h,temp_w])
                if temp_move not in self.states:
                    print("err in  get_reversi_pos: ",move)
                elif self.states[temp_move] == self.current_player:
                    break
                else:
                    reversi_pos.append(temp_move)

        return reversi_pos



    def has_a_winner(self):
        ret=False
        if len(self.get_all_avail_pos())==0:
            temp_player= self.current_player
            self.current_player= self.next_player
            if len(self.get_all_avail_pos())==0:
                ret=True # 双方都没有位置走了
            self.current_player = temp_player

        if ret:
            current_player_num=0 # 当前选手在棋盘上的棋子数量
            next_player_num=0
            for k, v in self.states.items():
                if v==self.current_player:
                    current_player_num+=1
                else:
                    next_player_num+=1
            if current_player_num>next_player_num:
                return True, self.current_player
            elif current_player_num==next_player_num:
                return True, -1
            else:
                return True, self.next_player

        return False, -1

    def game_end(self):
        """Check whether the game is ended or not"""
        win, winner = self.has_a_winner() # winner = -1 if there is no winner
        if win:
            return True, winner
        # elif not len(self.availables):
        #     return True, -1
        return False, -1

    def get_current_player(self):
        return self.current_player

    def board_check(self, a):
        if a >= 0 and a <= self.width-1:
            return True
        return False

    def is_avail_pos(self,location):
        h = location[0]
        w = location[1]
        if self.board_check(h) == False or self.board_check(w) == False :
            return False
        # already exist
        if self.location_to_move(location) in self.states: # 已经走过了
            return False

        for i in range (0,3):
            for j in range(0,3):
                temp_h= (location[0]-1+i)
                temp_w= (location[1]-1+j)
                temp_move= self.location_to_move([temp_h,temp_w])
                if temp_move not in self.states: #empty
                    continue
                elif self.states[temp_move] == self.current_player:
                    continue
                else: # another player
                    delta_h = temp_h - h
                    delta_w = temp_w - w
                    while self.board_check(temp_h+delta_h) and self.board_check(temp_w+delta_w):
                        temp_h += delta_h
                        temp_w += delta_w
                        temp_move= self.location_to_move([temp_h,temp_w])
                        if temp_move not in self.states:
                            break
                        elif self.states[temp_move] == self.current_player:
                            return True

        return False


    def get_all_avail_pos(self):

        posArr=[]
        for i in range(0,len(self.availables)):
            if self.is_avail_pos(self.move_to_location(self.availables[i])):
                posArr.append(self.availables[i])
        return posArr

