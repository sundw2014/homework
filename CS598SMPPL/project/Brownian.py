class RandomMotion(NiMC):
  def __init__(self, sigma):
    self.set_Theta([[1,2],[2,3]])
    self.sigma = sigma
    
  def is_unsafe(self, state):
    # return True if  |state|> 4
    if np.linalg.norm(state) > 4:
      return True 
    return False 


 def transition(self, state):
   # inc 2-d standard normal(0,sigma^2)
   inc=self.sigma*np.random.randn(2)
   state += inc
   return state # return the new state


# end of class


# class RandomMotion:
#   def __init__(self):
#     self.Theta =\
#      np.array([[1,2],[2,3]])
#     self.k = 10

#   @staticmethod
#   def is_unsafe(state):
#     if np.linalg.norm(state)>4:
#       return 1. # unsafe
#     return 0. #safe

#   @staticmethod
#   def transition(state):
#     sigma = 0.1
#     # increment is a 2-dimensional
#     # normally distributed vector
#     increment = sigma * np.random.randn(2)
#     state = state + increment
#     return state

### end of class

# initial_state_space \
# = np.array([[1,2],[2,3]])
# def is_unsafe(state):
#     if np.linalg.norm(state)>4:
#         return 1. # unsafe
#     return 0. #safe
# sigma = 0.1
# def step_forward(state):
#     # a normally distributed increment
#     increment = sigma * np.random.randn(2)
#     state = state + increment
#     return state
