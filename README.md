####Writing asynchronous code is never easy

Asynchronous callback force us to write code in [continuation passing style][CPS], which is tedious and error prone.  
consider simple naive synchronous function below:

[CPS]:http://en.wikipedia.org/wiki/Continuation-passing_style

```cpp
std::string md5(int fd) {
  char buf[BUFF_SIZE];
  md5_state_t ctx;
  md5_bytes digest[16];
  int bytes = 0;
  md5_init(&ctx);
  do {
    if(bytes)
      md5_append(&ctx, buf, bytes);
    bytes = read(fd, buf, BUFF_SIZE);
  } while(bytes);
  md5_finish(&ctx, digest);
  return std::string(digest, 16);
}

```

Which can be used sweetly as follow: 

```cpp
//determine filename...
std::string digest = md5(open(filename, O_RDONLY));
//use digest...
```

It's straight forward albeit error handling is ignored.
What's worse, it suffer from unresponsiveness and poor CPU utilization, just as other synchronous methd do.
To improve responsiveness, asynchronous methods like [aio] / [IOCP] must be introduced.
Assume someone kind enough already wrap all madness(signal, completion port, etc), and provide simple interface like:

[aio]:http://www.kernel.org/doc/man-pages/online/pages/man7/aio.7.html
[IOCP]:http://en.wikipedia.org/wiki/Input/output_completion_port

```cpp
typedef boost::shared_array<char> buf_type;
void readAsync(int fd, buf_type buf, ssize_t len, boost::function<void(int, buf_type, int)> completion_routine);
//where completion_routine defined as:
void completion_routine(int fd, buf_type buf, int bytes_read);
```
With this, ```md5(int fd)``` above can be rewritten like:

```cpp
void md5AsyncCallback(int fd, boost::function<void(const std::string&)>) {
  buf_type buf(new char[BUFF_SIZE]);
  boost::shared_ptr<md5_state_t> ctx(new md5_state_t);
  readAsync(fd, buf, BUFF_SIZE, boost::bind(continuation, ctx, callback, _1, _2, _3));
}

void continuation(boost::shared_ptr<md5_state_t> ctx,
                  boost::function<void(const std::string&)> callback,
                  int fd, buf_type buf, int bytes_read) {
  if(bytes_read > 0) {
    md5_append(ctx.get(), buf.get());
    readAsync(fd, buf, BUFF_SIZE, boost::bind(continuation, ctx, callback, _1, _2, _3));
  } else {
    md5_bytes digest[16]
    md5_finish(&ctx, digest);
    callback(std::string(digest, 16));
  }
}
```
To use async version, calling code also have to change:

```cpp
//determine filename...
md5AsyncCallback(open(filename, O_RDONLY), use_digst);
//return to event loop

void use_digst(const std::string &digest) {
  //use digest
}
```
Thanks to boost.bind, LOC is not bloating a lot, albeit **terrible** template parameters.
Despite cost of redudent boost.bind call every time readAsync is called, which can be eliminated by hand-written helper class,
cutting function into pieces is never a good idea, especially you are dealing with a sequence of steps.
This always gives birth to a bloat and error-prone class, which store all context needed in the sequence of workflow, and a very very complex state machine.

####[Delimited continuation][DC] is the sarvation.
[DC]:http://en.wikipedia.org/wiki/Delimited_continuation

Delimited continuation, also known as composable continuation, as its name implied is composable.
With delimited continuation, callback style code can be easily rewritten in a similar form of its synchronous equivilent.
In incoming Visual Studio 2012, C#/Visual Basic introduce new programming diagram: [Async/Await].
[Async/Await]:http://msdn.microsoft.com/en-us/library/hh191443%28v=vs.110%29.aspx

Though it's impossible to implement scheme like shift-reset with plain C++. 
It's possible to implement C# like Async/Await with [promise] and [coroutine].
[promise]:http://en.wikipedia.org/wiki/Futures_and_promises
[coroutine]:http://en.wikipedia.org/wiki/Coroutine

version readAsync looks like:
```cpp
  Promise<ssize_t> readAsync(int fd, boost::shared_array<char> buffer, size_t length, off_t offset);
```

And there is another important template function called await, we'll expain in detail later:

```cpp
  template<typename Ty_>
  Promise<Ty_> await(Promise<Ty_> pro);
```

Let's see what we can do with promise and coroutine:

```cpp
std::string md5Async(int fd) {
  boost::shared_array<char> buf(new char[BUFF_SIZE]); //1
  md5_state_t ctx;
  md5_bytes digest[16];
  ssize_t bytes = 0;
  int offset = 0;
  md5_init(&ctx);
  do {
    if(bytes)
      md5_append(&ctx, buf.get(), bytes);
    bytes = await(readAsync(fd, buf, BUFF_SIZE, offset)).result(); //2
    offset += bytes;
  } while(bytes);
  md5_finish(&ctx, digest);
  return std::string(digest, 16);
}
```
As you can see, md5Async is almost identical with original synchronous md5 function.
except:
  1. buf is redefined as shared_array, this is required by readAsync defination
  2. await on what ```readAsync``` return, and call ```result()``` method to extract bytes_read
To use promise-based async method, there are several ways:

In case all you have to do is calculate the md5 digest:

```cpp
  //determine filename...
  Promise<std::string> p = async((md5Async, open(filename, O_RDONLY))).promise();
  p.done(boost::bind(callback, p));
  //return to event loop

  void callback(Promise<std::string> p) {
    std::digest = p.result();
    //print the digest
  }
```

At first glance, there is no sinificant usage difference between this version and the callback-based version,
except md5Async itself become one solid function instead of a interface function and a callback.
Yes, doing one thing alone dosen't make async approach attractive,
but let's consider calculate md5 of that file is just part of a lengthy process,
we can then write every step inside that process in async style:

```cpp
void veryLengthyWork() {
  std::string filename = getFilenameAsync();
  int fd = openFileAsync(filename); //open a file on samba/nfs may take lots of time
  std::string digest = md5Async(fd);
  //etc.
}
```
Then, invoke the ```veryLengthyWork``` with ```async```:
```cpp
  async((veryLengthyWork()));
```
That's it! Every step in the process is written one after another in their logical order, easy to read, easy to maintain,
what's more important, have much better responsiveness and CPU utilization.  
Wait, the md5Async function is not overlapped properly yet!
we can issue another read to IO system before we spend CPU cycle in calculation!
Let's just do it:
```cpp
std::string md5Async(int fd) {
  boost::shared_array<char> reading(new char[BUFF_SIZE]), calculating(new char[BUFF_SIZE]);
  md5_state_t ctx;
  md5_bytes digest[16];
  int bytes = 0;
  ssize_t  offset = 0;
  Promise<ssize_t> rp;
  md5_init(&ctx);
  bytes = await(readAsync(fd, reading, BUFF_SIZE, offset)).result();
  do {
    std::swap(reading, calculating);
    offset += bytes;
    rp = readAsync(fd, buf, BUFF_SIZE, offset);
    if(bytes)
      md5_append(&ctx, reading.get(), bytes);
    bytes = await(rp).result();
  } while(bytes);
  md5_finish(&ctx, digest);
  return std::string(digest, 16);
}
```
Done! With another buffer and a little reordering,
this better version can issue read request to IO system before calculating already read portion of file.
Consider the md5AsyncCallback version, is possible to rewrite it in similar way without pain?
```cpp
void md5AsyncCallback(int fd, boost::function<void(const std::string&)>) {
  buf_type reading(new char[BUFF_SIZE]);
  buf_type calculating(new char[BUFF_SIZE]);
  boost::shared_ptr<md5_state_t> ctx(new md5_state_t);
  readAsync(fd, reading, BUFF_SIZE, boost::bind(continuation, ctx, calculating, callback, _1, _2, _3));
}

void continuation(boost::shared_ptr<md5_state_t> ctx,
                  boost::function<void(const std::string&)> callback, buf_type calculating,
                  int fd, buf_type reading, int bytes_read) {
  if(bytes_read > 0) {
    std::swap(reading, calculating);
    readAsync(fd, reading, BUFF_SIZE, boost::bind(continuation, ctx, calculating, callback, _1, _2, _3));
    md5_append(ctx.get(), reading.get());
  } else {
    md5_bytes digest[16]
    md5_finish(&ctx, digest);
    callback(std::string(digest, 16));
  }
}
```
Well...yes.
But how about adding time out to reading process?
Maybe user want a cancel button?
What's worse, maybe user can click your close button and you must cancel file access and free buffer right away?
I don't think it's easy any more.
But, all feature describe above can be done within a *SINGLE* do..while loop, for more detail, consult examples/md5async.

ok ok test
