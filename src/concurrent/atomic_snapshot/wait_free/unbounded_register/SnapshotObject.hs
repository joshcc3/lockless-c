{-# LANGUAGE TemplateHaskell #-}
import Control.Monad.State
import qualified Data.Set as S
import qualified Data.Map as M    
import Loop
import Control.Lens
import Control.Applicative

type Seq = Int
type Value = Int
type ProcId = Int
type M = (->)
type Snapshot = M ProcId Value
data ProcessState = P { _psval :: Value, _psseq :: Seq, _pssnap :: Snapshot } 
makeLenses ''ProcessState                  

type Register = ProcessState -> ProcessState

type Global = [ProcessState]

type Process a = State Global a

fromMap :: Ord a => b -> M.Map a b -> a -> b
fromMap b = (maybe b id .) . flip M.lookup

atomic = id

incSeq :: Register
incSeq = psseq %~ (+1)

updVal v = psval .~ v

setSnap s = pssnap .~ s

collect :: Process Snapshot
collect = undefined

procIds = return [1..10]

snap :: Process Snapshot
snap = do
  ids <- procIds
  let twoCollectCase = (liftA2 . liftA2 . liftA2) allEq collectStream (tailL collectStream)
          where allEq snap1 snap2 | all (liftA2 (==) snap1 snap2) ids = 1
                                  | otherwise = 0
      distinctSnapCount = (liftA2 . liftA2) (+) changed twoCollectCase
      threeDistinctCase = undefined -- (fmap . fmap . fmap) (>= 2) distinctSnapCount
      validSnaps = (liftA3 . liftA3) (choose ids) collectStream threeDistinctCase twoCollectCase
  consumeLoop validSnaps
    where
      choose ids c a b | a || b = Just c
                       | otherwise = Nothing
      collectStream = Loop collect collectStream
      changed = Loop (pure . pure $ 0) $ (liftA2 . liftA2 . liftA2) inc collectStream (tailL collectStream)
          where
            inc :: ProcessState -> ProcessState -> Int
            inc c1 c2 | _psseq c1 /= _psseq c2 = 1
                          | otherwise = 0

      
      
      

{-  for i in iterations:
      c1 <- collect
      c2 <- collect
      if seqs c1 == seqs c2:
         c2
      else:
          for procid in c2:
              if seq procid c1 != seq procid c2:
                 if moved procid:
                    return (snapshot c2)
                 else:
                     moved procid = true
-}          
              



update :: ProcId -> Int -> Process ()
update pid v = do
  snapshot <- snap
  ix pid %= atomic (incSeq . updVal v . setSnap snapshot)

