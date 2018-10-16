import qualified Data.Map as M
import qualified Data.List as L
import Control.Applicative

data Bit = One | Zero deriving (Eq, Ord, Show, Read)
type Val = (Bit, Bit)
data CheckerOp = Read1 | Read2 deriving (Eq, Ord, Show, Read)
data WriterOp = Write1 Val | Write2 Val deriving (Eq, Ord, Show, Read)


data Instr = C CheckerOp | W WriterOp deriving (Eq, Ord, Show, Read)

threadOps = [Write1, Write2] <*> validStates
checkerOps = [Read1, Read2]

processState :: Val -> [Instr] -> Val
processState v [] = v
processState v@(v1, _) (C Read1:i) = (v1, v2)
  where (_, v2) = processState v i
processState v@(_, v2) (C Read2:i) = (v1, v2)
  where (v1, _) = processState v i
processState v@(_, v2) (W (Write1 (v1, _)):i) = processState (v1, v2) i
processState v@(v1, _) (W (Write2 (_, v2)):i) = processState (v1, v2) i


atomicRead [x] = False
atomicRead [] = False
atomicRead (C Read1:C Read2:_) = True
atomicRead (C Read2:C Read1:_) = True
atomicRead (_:x) = atomicRead x

possibleOutcomes :: [[Instr]]
possibleOutcomes = filter (not . atomicRead) . L.permutations $ (C <$> checkerOps) ++ (W <$> threadOps) ++ (W <$> threadOps)

validStates = [zeroOne, oneZero]
zeroOne = (Zero, One)
oneZero = (One, Zero)

aggregate :: [Val] -> M.Map Val Int
aggregate = foldl f M.empty
  where f = flip $ M.alter (maybe (Just 1) (Just . (+1)))

test = processState oneZero t1 == (One, One) &&
       processState oneZero t2 == (One, One)
--	   &&
--       	   processState oneZero t3 == (One, One) &&
--       	   processState oneZero t4 == (One, One) &&
--       	   processState oneZero t5 == (One, Zero) &&
--       	   processState oneZero t6 == (One, Zero) &&
--       	   processState oneZero t7 == (Zero, One) &&
--       	   processState oneZero t8 == (Zero, One) &&
  where
    t1 = [W$  Write2 (undefined, One), C Read2]
    t2 = [W $ Write1 (One, Zero), W $ Write2 (Zero, Zero), W $ Write2 (One, One), C Read1, C Read2]


-- fromList [((One,One),709632), ((One,Zero),451584),((Zero,One),1032192),((Zero,Zero),709632)]

--main = do
--  print $ aggregate $ map (processState zeroOne) possibleOutcomes
--  print $ aggregate $ map (processState oneZero) possibleOutcomes