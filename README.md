## Usage

Initialize the SDK

```c++
StatsigUser user;
user.user_id = "a-user";

// Create your own instance
StatsigClient client;
client.Initialize("client-{YOUR_CLIENT_SDK_KEY}", user);

// or, use the shared instance
StatsigClient::Shared().Initialize("client-{YOUR_CLIENT_SDK_KEY}", user);
```


Check a Gate

```c++
if (client.CheckGate("a_gate")) {
  // show new feature
}

// or, use the shared instance

if (StatsigClient::Shared().CheckGate("a_gate")) {
  // show new feature
}
```


Get values from an Experiment

```c++
auto experiment = client.GetExperiment("an_experiment");

// or, use the shared instance

auto experiment = StatsigClient::Shared().GetExperiment("an_experiment");

// then access the params
  
std::cout << experiment.GetValue()["a_string_param"] << std::endl;

```

Getting experiment values from a Layer

```c++
auto layer = client.GetLayer("a_layer");

// or, use the shared instance

auto layer = StatsigClient::Shared().GetLayer("a_layer");

// then access the params

std::cout << layer.GetValue("a_string_param") << std::endl;
```


Logging an Event

```c++
unordered_map<string, string> metadata(
    {
        {"is_verified", "yes"}
    }
);


client.LogEvent({"my_custom_event", 1.23, metadata});

// or, use the shared instance

StatsigClient::Shared().LogEvent({"another_event", "string_values"});
```
