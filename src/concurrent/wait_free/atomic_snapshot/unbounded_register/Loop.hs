{-# LANGUAGE DeriveFunctor #-}
module Loop where

import Control.Applicative

data Loop a = Loop a (Loop a) deriving (Functor, Eq, Ord, Show, Read)

consumeLoop :: Monad m => Loop (m (Maybe a)) -> m a
consumeLoop (Loop x xs) = x >>= maybe (consumeLoop xs) return

instance Applicative Loop where
    pure x = xs
        where xs = Loop x xs
    Loop f n <*> Loop a n' = Loop (f a) $ n <*> n'

tailL (Loop a n) = n                             
